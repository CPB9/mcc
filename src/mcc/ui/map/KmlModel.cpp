/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "mcc/ui/map/KmlModel.h"
#include "mcc/ui/map/KmlSettings.h"
#include "mcc/ui/map/KmlUtils.h"
#include "mcc/ui/map/MapRect.h"

#include <QFile>
#include <QDebug>
#include <QStyle>
#include <QApplication>
#include <QFileDialog>
#include <QMenu>
#include <QMimeData>
#include <QClipboard>

#include "kml/dom.h"
#include "kml/engine.h"

namespace mcc {
namespace ui {
namespace map {

static kmldom::Document* parentDocument(kmldom::Element* element)
{
    if (element->IsA(kmldom::Type_Document)) {
        return static_cast<kmldom::Document*>(element);
    }
    auto parent = element->GetParent();
    if (!parent) {
        return 0;
    }
    return parentDocument(parent.get());
}

static kmldom::Container* parentContainer(kmldom::Element* element)
{
    auto parent = element->GetParent();
    if (parent->IsA(kmldom::Type_Container)) {
        return static_cast<kmldom::Container*>(parent.get());
    }
    return 0;
}

static inline kmldom::ElementPtr indexAsElement(const QModelIndex& index)
{
    return static_cast<kmldom::Element*>(index.internalPointer());
}

static inline kmldom::FeaturePtr indexAsFeature(const QModelIndex& index)
{
    return static_cast<kmldom::Feature*>(index.internalPointer());
}

void KmlModel::centerOn(const QModelIndex& index)
{
    if (!index.isValid()) {
        return;
    }
    kmldom::ElementPtr element = indexAsElement(index);
    if (element->IsA(kmldom::Type_Placemark)) {
        kmldom::PlacemarkPtr pm = kmldom::AsPlacemark(element);
        kmldom::CoordinatesPtr coordinates = KmlUtils::resolveCoordinates(pm.get());
        if (coordinates) {
            if (coordinates->get_coordinates_array_size() != 0) {
                kmlengine::Bbox bbox;
                kmlengine::GetCoordinatesBounds(coordinates, &bbox);
                core::LatLon latLon;
                bbox.GetCenter(&latLon.latitude, &latLon.longitude);
                emit centerOnRequested(latLon);
            }
        }
    }
}

template <typename F>
void KmlModel::insertElementAfterCurrent(F func)
{
    insertElementIntoIndex(_current, func);
}

template <typename F>
void KmlModel::insertElementBeforeIndex(const QModelIndex& index, F func)
{
    kmldom::FeaturePtr currentFeature = indexAsFeature(index);
    kmldom::Document* doc = parentDocument(currentFeature.get());
    kmldom::Container* cont;
    std::size_t pos;
    QModelIndex idx;
    cont = parentContainer(currentFeature.get());
    pos = index.row();
    idx = index.parent();
    beginInsertRows(idx, pos, pos);
    kmldom::FeaturePtr feature = func(doc);
    cont->insert_feature_at(pos, feature);
    endInsertRows();
    notifyAddition(doc, feature);
}

template <typename F>
void KmlModel::insertElementIntoIndex(const QModelIndex& index, F func)
{
    kmldom::FeaturePtr currentFeature = indexAsFeature(index);
    kmldom::Document* doc = parentDocument(currentFeature.get());
    kmldom::Container* cont;
    std::size_t pos;
    QModelIndex idx;
    if (currentFeature->IsA(kmldom::Type_Container)) {
        cont = static_cast<kmldom::Container*>(currentFeature.get());
        pos = cont->get_feature_array_size();
        idx = index;
    } else {
        cont = parentContainer(currentFeature.get());
        pos = index.row();
        idx = index.parent();
    }
    beginInsertRows(idx, pos, pos);
    kmldom::FeaturePtr feature = func(doc);
    cont->insert_feature_at(pos, feature);
    endInsertRows();
    notifyAddition(doc, feature);
}

static const QString mimeTypeStr = "mcc/kml";

QMimeData* KmlModel::mimeDataOfIndex(const QModelIndex& index) const
{
    kmldom::FeaturePtr feature = indexAsFeature(index);
    std::string result = kmldom::SerializeRaw(feature);
    QByteArray data = QByteArray(result.c_str());
    QMimeData* mimeData = new QMimeData;
    mimeData->setData(mimeTypeStr, data);
    return mimeData;
}

void KmlModel::copyCurrentToClipboard() const
{
    QMimeData* mimeData = mimeDataOfIndex(_current);
    QApplication::clipboard()->setMimeData(mimeData);
}

void KmlModel::removeIndex(const QModelIndex& index)
{
    kmldom::FeaturePtr feature = indexAsFeature(index);
    kmldom::ContainerPtr cont = KmlUtils::parentContainer(feature.get());
    if (!cont) {
        return;
    }
    int i = cont->get_index_of_feature(feature);
    if (i == -1) {
        return;
    }
    beginRemoveRows(this->parent(index), i, i);
    notifyDeletion(feature.get());
    cont->DeleteFeatureAt(i);
    endRemoveRows();
    //_removeAction->setDisabled(true);
}

void KmlModel::removeCurrent()
{
    removeIndex(_current);
}

kmldom::ElementPtr KmlModel::mimeDataToElement(const QMimeData* mimeData) const
{
    QByteArray data = mimeData->data(mimeTypeStr);
    if (data.isEmpty()) {
        return nullptr;
    }
    std::string src = data.constData();
    std::string errors;
    kmldom::ElementPtr element = kmldom::Parse(src, &errors);
    if (!errors.empty()) {
        qDebug() << errors.c_str();
    }
    return element;
}

KmlModel::KmlModel(const MapRectConstPtr& rect, QWidget* parent)
    : QAbstractItemModel(parent)
    , _rect(rect)
{
    kmldom::KmlFactory* factory = kmldom::KmlFactory::GetFactory();
    _root = factory->CreateKml();
    _visibleRoot = factory->CreateDocument();
    _settings = new KmlSettings(parent);
    _settings->setWindowFlags(_settings->windowFlags() | Qt::Tool);
    _openAction = new QAction("Открыть...", this);
    _saveAction = new QAction("Сохранить как...", this);
    _createEmptyAction = new QAction("Очистить", this);
    _saveAction->setEnabled(false);
    _propertiesAction = new QAction("Параметры", this);
    _copyAction = new QAction("Копировать", this);
    _copyAction->setEnabled(false);
    _cutAction = new QAction("Вырезать", this);
    _cutAction->setEnabled(false);
    _pasteAction = new QAction("Вставить", this);
    _pasteAction->setEnabled(false);
    _removeAction = new QAction("Удалить", this);
    _removeAction->setEnabled(false);
    _openMenu = new QMenu;
    initOpenMenu(_openMenu);
    _fullMenu = new QMenu;
    QMenu* addMenu = _fullMenu->addMenu("Добавить");
    _fullMenu->addSeparator();
    initOpenMenu(_fullMenu);
    _fullMenu->addSeparator();
    QAction* pasteFromFile = _fullMenu->addAction("Вставить элемент из файла...");
    pasteFromFile->setEnabled(false);
    QAction* saveToFile = _fullMenu->addAction("Сохранить элемент в файл...");
    saveToFile->setEnabled(false);
    _fullMenu->addSeparator();
    _addFolderAction = addMenu->addAction("Папку");
    _addPointAction = addMenu->addAction("Флаг");
    _addPolylineAction = addMenu->addAction("Ломаную");
    _addPolygonAction = addMenu->addAction("Полигон");
    _addFolderAction->setEnabled(false);
    _addPointAction->setEnabled(false);
    _addPolygonAction->setEnabled(false);
    _addPolylineAction->setEnabled(false);
    _propertiesAction->setEnabled(false);
    initMenu(_fullMenu);
    _simpleMenu = new QMenu;
    initMenu(_simpleMenu);

    connect(_addFolderAction,   &QAction::triggered, this, &KmlModel::onFolderAdd);
    connect(_addPointAction,    &QAction::triggered, this, &KmlModel::onPointAdd);
    connect(_addPolylineAction, &QAction::triggered, this, &KmlModel::onPolylineAdd);
    connect(_addPolygonAction,  &QAction::triggered, this, &KmlModel::onPolygonAdd);

    connect(_propertiesAction, &QAction::triggered, _settings, [this]() {
        kmldom::Feature* feature = static_cast<kmldom::Feature*>(_current.internalPointer());
        _settings->showSettings(parentDocument(feature), static_cast<kmldom::Placemark*>(feature));
    });

    connect(_createEmptyAction, &QAction::triggered, this, [this]() { openEmpty(); });

    connect(_openAction, &QAction::triggered, this, [this]() {
        QString path = QFileDialog::getOpenFileName(0, "Открыть KML", "", "KML-файлы (*.kml)");
        if (path.isEmpty()) {
            return;
        }
        open(path);
    });

    connect(_saveAction, &QAction::triggered, this, [this]() {
        QString path = QFileDialog::getSaveFileName(0, "Сохранить KML", "", "KML-файлы (*.kml)");
        if (path.isEmpty()) {
            return;
        }
        if (QFileInfo(path).suffix() != "kml") {
            if (path[path.size() - 1] != '.') {
                path.append(".kml");
            } else {
                path.append("kml");
            }
        }
        save(path);
    });

    connect(_cutAction, &QAction::triggered, this, [this]() {
        copyCurrentToClipboard();
        removeCurrent();
    });

    connect(_removeAction, &QAction::triggered, this, [this]() { removeCurrent(); });
    connect(_copyAction, &QAction::triggered, this, [this]() { copyCurrentToClipboard(); });
    connect(_pasteAction, &QAction::triggered, this, [this]() {
        kmldom::ElementPtr element = mimeDataToElement(QApplication::clipboard()->mimeData());
        if (!element) {
            return;
        }
        if (!KmlUtils::isEditableElement(element.get())) {
            return;
        }
        insertElementAfterCurrent([element](kmldom::Document* parent) {
            (void)parent;
            return kmldom::AsFeature(element);
        });
    });
    connect(_settings, &KmlSettings::elementChanged, this, &KmlModel::elementChanged);
    openEmpty();
    selectIndex(QModelIndex());
}

KmlModel::~KmlModel()
{
    delete _openMenu;
    delete _fullMenu;
    delete _simpleMenu;
}

void KmlModel::initOpenMenu(QMenu* menu)
{
    menu->addAction(_createEmptyAction);
    menu->addAction(_openAction);
    menu->addAction(_saveAction);
}

void KmlModel::initMenu(QMenu* menu)
{
    menu->addAction(_copyAction);
    menu->addAction(_cutAction);
    menu->addAction(_pasteAction);
    menu->addAction(_removeAction);
    menu->addSeparator();
    menu->addAction(_propertiesAction);
}

void KmlModel::selectIndex(const QModelIndex& idx)
{
    QModelIndex current = idx;
    if (!current.isValid()) {
        current = index(0, 0);
    }
    kmldom::Element* element = static_cast<kmldom::Element*>(current.internalPointer());
    _propertiesAction->setEnabled(element->IsA(kmldom::Type_Feature));
    _removeAction->setEnabled(current.isValid() && element != _visibleRoot);
    _copyAction->setEnabled(current.isValid());
    _cutAction->setEnabled(current.isValid());
    _pasteAction->setEnabled(QApplication::clipboard()->mimeData()->hasFormat(mimeTypeStr));
    _addFolderAction->setEnabled(current.isValid());
    _addPointAction->setEnabled(current.isValid());
    _addPolygonAction->setEnabled(current.isValid());
    _addPolylineAction->setEnabled(current.isValid());
    emit elementSelected(parentDocument(element), element);
    _current = current;
}

QMenu* KmlModel::fullMenu()
{
    return _fullMenu;
}

void KmlModel::showContextMenu(const QModelIndex& index, const QPoint& point)
{
    _current = index;
    if (_current.isValid()) {
        _fullMenu->exec(point);
    } else {
        _openMenu->exec(point);
    }
}

void KmlModel::openEmpty()
{
    kmldom::KmlFactory* factory = kmldom::KmlFactory::GetFactory();
    kmldom::KmlPtr empty = factory->CreateKml();
    kmldom::DocumentPtr emptyDoc = factory->CreateDocument();
    emptyDoc->set_name("root");
    empty->set_feature(emptyDoc);
    std::string data = kmldom::SerializePretty(empty);
    std::string errors;
    _file.reset(kmlengine::KmlFile::CreateFromParse(data, &errors));
    _saveAction->setEnabled(true);

    beginResetModel();
    _root = _file->get_root();
    _visibleRoot = kmldom::AsKml(_root)->get_feature();
    endResetModel();
    selectIndex(QModelIndex());
}

void KmlModel::open(const QString& path)
{
    QFile file(path);
    file.open(QIODevice::ReadOnly);
    QByteArray qdata = file.readAll();
    std::string data = qdata.constData();
    std::string errors;
    _file.reset(kmlengine::KmlFile::CreateFromParse(data, &errors));
    kmldom::ElementPtr root = _file->root();
    if (!root) {
        _file.release();
        return;
    }
    _saveAction->setEnabled(true);
    _addFolderAction->setEnabled(true);
    _addPointAction->setEnabled(true);
    _addPolygonAction->setEnabled(true);
    _addPolylineAction->setEnabled(true);
    _propertiesAction->setEnabled(true);
    beginResetModel();
    if (root->IsA(kmldom::Type_kml)) {
        kmldom::KmlPtr kml = kmldom::AsKml(root);
        if (kml->has_feature()) {
            _visibleRoot = kml->get_feature();
        } else {
            _visibleRoot = 0;
        }
        _root = root;
    } else {
        kmldom::KmlFactory* factory = kmldom::KmlFactory::GetFactory();
        kmldom::KmlPtr kml = factory->CreateKml();
        kml->set_feature(kmldom::AsFeature(root)); //HACK
        _root = kml;
        _visibleRoot = root;
    }
    endResetModel();
    selectIndex(QModelIndex());
}

void KmlModel::save(const QString& path)
{
    std::string out;
    _file->SerializeToString(&out);
    QFile file(path);
    file.open(QIODevice::WriteOnly);
    file.write(out.c_str());
}

kmldom::ElementPtr KmlModel::root()
{
    return _visibleRoot;
}

kmldom::DocumentPtr KmlModel::rootDocument()
{
    return parentDocument(_visibleRoot.get());
}

int KmlModel::rowCount(const QModelIndex& parent) const
{
    kmldom::ElementPtr element;
    if (!parent.isValid()) {
        element = _root;
    } else {
        element = indexAsElement(parent);
    }

    if (element->IsA(kmldom::Type_kml)) {
        kmldom::KmlPtr kml = kmldom::AsKml(element);
        if (kml->has_feature()) {
            return 1;
        } else {
            return 0;
        }
    } else if (element->IsA(kmldom::Type_Container)) {
        kmldom::ContainerPtr container = kmldom::AsContainer(element);
        return container->get_feature_array_size();
    }
    return 0;
}

int KmlModel::columnCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent);
    return 2;
}

QVariant KmlModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid()) {
        return QVariant();
    }

    kmldom::ElementPtr element = indexAsElement(index);
    if (!KmlUtils::isEditableElement(element.get())) {
        return QVariant();
    }

    kmldom::FeaturePtr feature = kmldom::AsFeature(element);
    if (role == Qt::DisplayRole || role == Qt::EditRole) {
        if (index.column() == 0) {
            if (feature->has_name()) {
                return feature->get_name().c_str();
            } else {
                return "<no name>";
            }
        } else {
            if (feature->has_description()) {
                return feature->get_description().c_str();
            }
        }

    } else if (role == Qt::DecorationRole && index.column() == 0) {
        if (element->IsA(kmldom::Type_Folder)) {
            return QApplication::style()->standardIcon(QStyle::SP_DirIcon);
        } else if (element->IsA(kmldom::Type_Document)) {
            return QApplication::style()->standardIcon(QStyle::SP_FileIcon);
        }
    } else if (role == Qt::CheckStateRole && index.column() == 0) {
        if (feature->get_visibility()) {
            return Qt::Checked;
        } else {
            return Qt::Unchecked;
        }
    } else if (role == Qt::ToolTipRole && index.column() == 1 && feature->has_description()) {
        return feature->get_description().c_str();
    }

    return QVariant();
}

QVariant KmlModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
        if (section == 0) {
            return "Название";
        } else if (section == 1) {
            return "Описание";
        }
    }
    return QVariant();
}

QModelIndex KmlModel::index(int row, int column, const QModelIndex& parent) const
{
    if (!hasIndex(row, column, parent)) {
        return QModelIndex();
    }

    kmldom::ElementPtr element;
    if (!parent.isValid()) {
        element = _root;
    } else {
        element = indexAsElement(parent);
    }
    if (element->IsA(kmldom::Type_kml)) {
        kmldom::KmlPtr kml = kmldom::AsKml(element);
        if (kml->has_feature() && row == 0) {
            return createIndex(row, column, kml->get_feature().get());
        } else {
            return QModelIndex();
        }
    } else if (element->IsA(kmldom::Type_Container)) {
        kmldom::ContainerPtr container = kmldom::AsContainer(element);
        kmldom::FeaturePtr feature = container->get_feature_array_at(row);
        if (feature) {
            return createIndex(row, column, feature.get());
        }
    }
    return QModelIndex();
}

QModelIndex KmlModel::parent(const QModelIndex& child) const
{
    if (!child.isValid()) {
        return QModelIndex();
    }

    kmldom::ElementPtr childItem = indexAsElement(child);
    kmldom::ElementPtr parentItem = childItem->GetParent();

    if (!parentItem->IsA(kmldom::Type_Feature) || parentItem.get() == _root) {
        return QModelIndex();
    }

    kmldom::FeaturePtr parentFeature = kmldom::AsFeature(parentItem);
    kmldom::ElementPtr parentParentItem = parentItem->GetParent();

    if (parentParentItem) {
        if (parentParentItem->IsA(kmldom::Type_Container)) {
            kmldom::ContainerPtr container = kmldom::AsContainer(parentParentItem);
            int index = container->get_index_of_feature(parentFeature);
            if (index != -1) {
                return createIndex(index, 0, parentItem.get());
            }
        }
    }

    return createIndex(0, 0, parentItem.get());
}

Qt::ItemFlags KmlModel::flags(const QModelIndex& index) const
{
    if (!index.isValid() && _visibleRoot->IsA(kmldom::Type_Container)) {
        return Qt::ItemIsDropEnabled;
    }

    Qt::ItemFlags flags = Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled;

    if (index.column() == 0) {
        flags |= Qt::ItemIsUserCheckable;
    }

    return flags;
}

void KmlModel::notifyDeletion(kmldom::Element* element)
{
    if (element->IsA(kmldom::Type_Container)) {
        kmldom::Container* container = static_cast<kmldom::Container*>(element);
        for (std::size_t i = 0; i < container->get_feature_array_size(); i++) {
            kmldom::Feature* subFeature = container->get_feature_array_at(i).get();
            notifyDeletion(subFeature);
        }
    } else {
        emit elementRemoved(parentDocument(element), element);
    }
}

void KmlModel::notifyAddition(kmldom::Document* parent, const kmldom::ElementPtr& element)
{
    if (element->IsA(kmldom::Type_Container)) {
        kmldom::Container* container = static_cast<kmldom::Container*>(element.get());
        for (std::size_t i = 0; i < container->get_feature_array_size(); i++) {
            kmldom::Feature* subFeature = container->get_feature_array_at(i).get();
            notifyAddition(parent, subFeature);
        }
    } else {
        emit elementAdded(parentDocument(element.get()), element);
    }
}

static QVector<int> checkStateRoles = {Qt::CheckStateRole};

void KmlModel::setParentsVisible(const QModelIndex& index)
{
    QModelIndex parentIndex = parent(index);
    if (parentIndex.isValid()) {
        kmldom::ElementPtr parent = indexAsElement(parentIndex);
        if (parent->IsA(kmldom::Type_Feature)) {
            kmldom::FeaturePtr feature = kmldom::AsFeature(parent);
            feature->set_visibility(true);
            emit dataChanged(parentIndex, parentIndex, checkStateRoles);
        }
        setParentsVisible(parentIndex);
    }
}

void KmlModel::setVisibility(const QModelIndex& index, bool visibility)
{
    if (visibility) {
        setParentsVisible(index);
    }
    setChildrenVisibility(index, visibility);
}

void KmlModel::setChildrenVisibility(const QModelIndex& index, bool visibility)
{
    kmldom::ElementPtr element = indexAsElement(index);
    if (!KmlUtils::isEditableElement(element.get())) {
        return;
    }
    kmldom::FeaturePtr feature = kmldom::AsFeature(element);
    feature->set_visibility(visibility);
    emit visibilityChanged(parentDocument(feature.get()), feature.get(), visibility, index == _current);
    if (feature->IsA(kmldom::Type_Container)) {
        kmldom::ContainerPtr container = kmldom::AsContainer(feature);
        for (std::size_t i = 0; i < container->get_feature_array_size(); i++) {
            kmldom::FeaturePtr subFeature = container->get_feature_array_at(i).get();
            setChildrenVisibility(createIndex(i, 0, subFeature.get()), visibility);
        }
    }
    emit dataChanged(index, index, checkStateRoles);
}

bool KmlModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
    if (!index.isValid())
        return false;

    kmldom::ElementPtr element = indexAsElement(index);
    if (!element->IsA(kmldom::Type_Feature)) {
        return false;
    }
    kmldom::FeaturePtr feature = kmldom::AsFeature(element);
    if (role == Qt::EditRole) {
        QByteArray str = value.toString().toUtf8();
        if (index.column() == 0) {
            feature->set_name(str.constData());
        }
    } else if (role == Qt::CheckStateRole && index.column() == 0) {
        if (element->IsA(kmldom::Type_Feature)) {
            setVisibility(index, value.toBool());
            return true;
        }
    }
    return false;
}

bool KmlModel::removeRows(int row, int count, const QModelIndex& parent)
{
    if (count < 1) {
        return false;
    }
    removeIndex(index(row, 0, parent));
    return true;
}

Qt::DropActions KmlModel::supportedDragActions() const
{
    return Qt::MoveAction | Qt::TargetMoveAction;
}

Qt::DropActions KmlModel::supportedDropActions() const
{
    return Qt::MoveAction | Qt::TargetMoveAction;
}

QStringList KmlModel::mimeTypes() const
{
    QStringList types;
    types << mimeTypeStr;
    return types;
}

QMimeData* KmlModel::mimeData(const QModelIndexList& indexes) const
{
    assert(!indexes.isEmpty());
    return mimeDataOfIndex(indexes[0]);
}

bool KmlModel::canDropMimeData(const QMimeData* data, Qt::DropAction action, int row, int column,
                               const QModelIndex& parent) const
{
    (void)action;
    (void)row;
    (void)column;
    (void)parent;
    return data->hasFormat(mimeTypeStr);
}

bool KmlModel::dropMimeData(const QMimeData* data, Qt::DropAction action, int row, int column,
                            const QModelIndex& parent)
{
    if (action == Qt::IgnoreAction) {
        return true;
    }
    if (row == -1 && column == -1) {
        QModelIndex index;
        if (!parent.isValid() && _visibleRoot->IsA(kmldom::Type_Container)) {
            index = this->createIndex(0, 0, _visibleRoot.get());
        } else {
            index = parent;
        }
        insertElementIntoIndex(index,
                               [this, data](kmldom::Document*) { return kmldom::AsFeature(mimeDataToElement(data)); });
    } else {
        QModelIndex index;
        if (row >= rowCount(parent)) {
            index = parent;
        } else {
            index = this->index(row, 0, parent);
        }
        insertElementBeforeIndex(
            index, [this, data](kmldom::Document*) { return kmldom::AsFeature(mimeDataToElement(data)); });
    }
    return true;
}

void KmlModel::onFolderAdd()
{
    insertElementAfterCurrent(KmlUtils::createEmptyDirectory);
}

void KmlModel::onPointAdd()
{
    insertElementAfterCurrent([this](kmldom::Document* doc) {
        core::LatLon latLon = _rect->centerLatLon();
        kmlbase::Vec3 vec3 = kmlbase::Vec3(latLon.longitude, latLon.latitude);
        return KmlUtils::createEmptyPoint(doc, vec3);
    });
}

void KmlModel::onPolylineAdd()
{
    insertElementAfterCurrent(KmlUtils::createEmptyPolyline);
}

void KmlModel::onPolygonAdd()
{
    insertElementAfterCurrent(KmlUtils::createEmptyPolygon);
}

}
}
}
