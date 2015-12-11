/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#pragma once

#include "mcc/ui/core/Structs.h"
#include "mcc/ui/map/Ptr.h"

#include <QAbstractItemModel>

#include <kml/dom/kml_ptr.h>

#include <memory>
#include <vector>

namespace kmldom {
class Element;
class Document;
}

namespace kmlengine {
class KmlFile;
}

class QMenu;
class QAction;
class QWidget;

namespace mcc {
namespace ui {
namespace map {

class KmlSettings;

class KmlModel : public QAbstractItemModel {
    Q_OBJECT
public:
    KmlModel(const MapRectConstPtr& rect, QWidget* parent = 0);
    ~KmlModel() override;

    void open(const QString& path);
    void openEmpty();
    void save(const QString& path);

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    int columnCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex& child) const override;
    Qt::ItemFlags flags(const QModelIndex& index) const override;
    bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole) override;
    bool removeRows(int row, int count, const QModelIndex& parent = QModelIndex()) override;
    Qt::DropActions supportedDragActions() const override;
    Qt::DropActions supportedDropActions() const override;
    QStringList mimeTypes() const override;
    QMimeData* mimeData(const QModelIndexList& indexes) const override;
    bool canDropMimeData(const QMimeData* data, Qt::DropAction action, int row, int column,
                         const QModelIndex& parent) const override;
    bool dropMimeData(const QMimeData* data, Qt::DropAction action, int row, int column,
                      const QModelIndex& parent) override;

    void centerOn(const QModelIndex& index);
    void showContextMenu(const QModelIndex& index, const QPoint& point);
    QMenu* fullMenu();
    void selectIndex(const QModelIndex& index);

    kmldom::ElementPtr root();
    kmldom::DocumentPtr rootDocument();
public slots:
    void onFolderAdd();
    void onPointAdd();
    void onPolylineAdd();
    void onPolygonAdd();

signals:
    void visibilityChanged(kmldom::Document* parent, const kmldom::ElementPtr& element, bool isVisible,
                           bool isSelected = false);
    void elementChanged(kmldom::Document* parent, const kmldom::ElementPtr& element);
    void elementAdded(kmldom::Document* parent, const kmldom::ElementPtr& element);
    void elementRemoved(kmldom::Document* parent, kmldom::Element* element);
    void elementSelected(kmldom::Document* parent, kmldom::Element* element);
    void centerOnRequested(const core::LatLon& latLon);

private:
    QMimeData* mimeDataOfIndex(const QModelIndex& index) const;
    template <typename F>
    void insertElementIntoIndex(const QModelIndex& index, F func);
    template <typename F>
    void insertElementBeforeIndex(const QModelIndex& index, F func);
    template <typename F>
    void insertElementAfterCurrent(F func);
    kmldom::ElementPtr mimeDataToElement(const QMimeData* mimeData) const;
    void copyCurrentToClipboard() const;
    void removeIndex(const QModelIndex& index);
    void removeCurrent();
    void notifyDeletion(kmldom::Element* element);
    void notifyAddition(kmldom::Document* parent, const kmldom::ElementPtr& element);
    void initMenu(QMenu* menu);
    void initOpenMenu(QMenu* menu);
    void setParentsVisible(const QModelIndex& index);
    void setChildrenVisibility(const QModelIndex& index, bool visibility);
    void setVisibility(const QModelIndex& index, bool visibility);
    kmldom::ElementPtr _root;
    kmldom::ElementPtr _visibleRoot;
    std::unique_ptr<kmlengine::KmlFile> _file;
    KmlSettings* _settings;
    QAction* _openAction;
    QAction* _createEmptyAction;
    QAction* _saveAction;
    QAction* _copyAction;
    QAction* _cutAction;
    QAction* _pasteAction;
    QAction* _removeAction;
    QAction* _propertiesAction;
    QAction* _addFolderAction;
    QAction* _addPointAction;
    QAction* _addPolylineAction;
    QAction* _addPolygonAction;
    QMenu* _openMenu;
    QMenu* _fullMenu;
    QMenu* _simpleMenu;
    QModelIndex _current;
    MapRectConstPtr _rect;
};
}
}
}
