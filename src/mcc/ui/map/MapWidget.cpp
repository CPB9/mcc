/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "mcc/ui/map/MapWidget.h"

#include "mcc/ui/core/DeviceManager.h"
#include "mcc/ui/core/Settings.h"
#include "mcc/ui/map/CacheStackModel.h"
#include "mcc/ui/map/CacheStackView.h"
#include "mcc/ui/map/KmlModel.h"
#include "mcc/ui/map/MapWidgetAnimator.h"
#include "mcc/ui/map/layers/LayerGroup.h"
#include "mcc/ui/map/layers/MapLayer.h"
#include "mcc/ui/map/MapRect.h"
#include "mcc/ui/map/MapSlider.h"
#include "mcc/ui/map/OmcfCache.h"
#include "mcc/ui/map/OnlineCache.h"
#include "mcc/ui/map/StackCache.h"
#include "mcc/ui/map/StaticMapType.h"
#include "mcc/ui/map/WebMapProperties.h"
#include "mcc/ui/map/layers/DeviceLayer.h"
#include "mcc/ui/map/layers/HomeLayer.h"

#include <QActionGroup>
#include <QDebug>
#include <QMenu>
#include <QPainter>
#include <QPaintEvent>
#include <QPushButton>
#include <QSize>
#include <QTimer>
#include <QSlider>
#include <QToolBar>
#include <QToolButton>

#include <cmath>
#include <array>

namespace mcc {
namespace ui {
namespace map {

static const int subWidgetMargin = 10;

KmlModel* MapWidget::kmlModel() const
{
    return _kmlModel;
}

MapWidget::MapWidget(core::DeviceManager* manager, QWidget* parent)
    : MCC_MAP_WIDGET_BASE(parent)
    , _currentMousePos(0, 0)
    , _isMovingViewport(false)
    , _wasOnline(false)
{
#ifdef MCC_USE_OPENGL
    QGLWidget::setFormat(QGLFormat(QGL::SampleBuffers));
#endif
    setAttribute(Qt::WA_NoSystemBackground);
    setAttribute(Qt::WA_OpaquePaintEvent);
    setMouseTracking(true);
    setMinimumSize(400, 300);
    _animator.reset(new MapWidgetAnimator(this));
    _rect = MapRect::create(this);
    _constRect = _rect;

    KmlModel* kmlModel = new KmlModel(_constRect, this);
    _kmlModel = kmlModel;
    createStack();
    createLayers(manager, kmlModel);
    createMapWidgets();

    connect(core::Settings::instance(), &core::Settings::mapCachePathChanged, this, [this](const QString& path) {
        _cacheStackModel->setOnlineCachePath(path);
        _layers->mapLayer()->reload();
    });

    connect(kmlModel, &KmlModel::centerOnRequested, this,
            [this](const core::LatLon& latLon) { centerOn(latLon.latitude, latLon.longitude); });
    resize(size());
    adjustChildren();
#if MCC_USE_OPENGL
    connect(this, &MapWidget::mapNeedsUpdate, this, [this]() { update(); }, Qt::QueuedConnection);
#endif
}

void MapWidget::updateMap()
{
#if MCC_USE_OPENGL
    emit mapNeedsUpdate();
#else
    update();
#endif
}

void MapWidget::createLayers(core::DeviceManager* manager, KmlModel* kmlModel)
{
    _layers = std::make_shared<LayerGroup>(_cacheStackModel->onlineCache(StaticMapType::GoogleMap), manager, kmlModel,
                                           _rect);
    connect(_layers.get(), &Layer::sceneUpdated, this, [this]() { updateMap(); });
}

void MapWidget::createStack()
{
    _cacheStackModel = new CacheStackModel();
    _cacheStackView = new CacheStackView(_cacheStackModel);
    _cacheStackView->setEnabled(false);
    _cacheStackModel->selectOnlineMap(StaticMapType::GoogleMap);
    connect(_cacheStackView, &CacheStackView::cacheUpdated, this, [this]() {
        const StackCachePtr& cache = _cacheStackModel->stack();
        onCacheChanged(cache);
        updateMap();
    });
    //     connect(_cacheStackView, &CacheStackView::clicked, this, [this]() {
    //         _wasOnline = _onlineButton->isChecked();
    //         _cacheStackView->setEnabled(true);
    //         _onlineButton->setChecked(false);
    //         const StackCachePtr& cache = _cacheStackModel->stack();
    //         onCacheChanged(cache);
    //         updateMap();
    //     });
}

void MapWidget::createMapWidgets()
{
    _contextMenu = new QMenu;
    _layerMenu = _contextMenu->addMenu("Карта");
    _slider = new MapSlider(this);

    connect(_slider, &MapSlider::valueChanged, this, &MapWidget::setZoomLevel);
    _layerActions = new QActionGroup(this);
    _layerButton = new QPushButton(this);
    _layerButton->setText("Google карта");
    _layerButton->setMenu(_layerMenu);
    _onlineMenu = new QMenu("Оффлайн");
    _onlineButton = new QPushButton(this);
    _onlineButton->setMenu(_onlineMenu);
    _offlineAction = _onlineMenu->addAction("Оффлайн");
    _onlineAction = _onlineMenu->addAction("Онлайн");
    _stackAction = _onlineMenu->addAction("Стек карт");
    _onlineActions = new QActionGroup(this);
    for (QAction* action : std::array<QAction*, 3>{{_offlineAction, _onlineAction, _stackAction}}) {
        action->setCheckable(true);
        _onlineActions->addAction(action);
    }
    _offlineAction->setChecked(true);
    _onlineButton->setText(_offlineAction->text());
    //     _onlineButton->setStyleSheet("QPushButton          { color: red; }"
    //                                  "QPushButton:disabled { color: grey;  }"
    //                                  "QPushButton:pressed  { color: darkgreen; }"
    //                                  "QPushButton:checked  { color: darkgreen; }");

    const std::map<const char*, std::map<const char*, StaticMapType>> menues
        = {{"Google",
            {{"Карта", StaticMapType::GoogleMap},
             {"Ландшафт", StaticMapType::GoogleLandscape},
             {"Спутник", StaticMapType::GoogleSatellite}}},
           {"Yandex",
            {{"Карта", StaticMapType::YandexMap},
             {"Народная", StaticMapType::YandexNarodnaya},
             {"Спутник", StaticMapType::YandexSatellite}}},
           {"OpenStreetMap",
            {{"Basic", StaticMapType::OsmBasic},
             {"MapQuest", StaticMapType::OsmMapQuest},
             {"MapQuest Sat", StaticMapType::OsmMapQuestSat},
             {"Thunderforest Landscape", StaticMapType::OsmThunderforestLandscape}}}
        };
    for (const auto& serviceMaps : menues) {
        QMenu* serviceMenu = _layerMenu->addMenu(serviceMaps.first);
        for (const auto& nameAndType : serviceMaps.second) {
            QAction* action = serviceMenu->addAction(nameAndType.first);
            action->setCheckable(true);
            _layerActions->addAction(action);
            _typeToAction.emplace(nameAndType.second, action);
        }
    }

    _staticMapType = core::Settings::instance()->staticMapType();
    _typeToAction[_staticMapType]->setChecked(true); // TODO: читать из настроек

    auto setType = [this](StaticMapType type) {
        adjustChildren();
        //adjustChildren();
        const OnlineCachePtr& cache = _cacheStackModel->onlineCache(type);
        _layerButton->setText(cache->name());
        _layerButton->adjustSize();
        onCacheChanged(cache);
        _cacheStackModel->selectOnlineMap(type);
        _staticMapType = type;
        core::Settings::instance()->setStaticMapType(type);
        updateMap();
    };


    for (const auto& kv : _typeToAction) {
        StaticMapType type = kv.first;
        connect(kv.second, &QAction::triggered, [this, setType, type]() {
            setType(type);
        });
    }

    connect(_onlineAction, &QAction::triggered, this, [this]() {
        onOnlineModeChanged(_onlineAction->text(), true, true, false);
        core::Settings::instance()->setMapMode(core::MapMode::Online);
    });

    connect(_offlineAction, &QAction::triggered, this, [this]() {
        onOnlineModeChanged(_offlineAction->text(), false, true, false);
        core::Settings::instance()->setMapMode(core::MapMode::Offline);
    });

    connect(_stackAction, &QAction::triggered, this, [this]() {
        onOnlineModeChanged(_stackAction->text(), false, false, true);
        core::Settings::instance()->setMapMode(core::MapMode::Stack);
    });

    _rulerModeButton = createToolBarButton("Линейка", ":/resources/ruler.png", MapMode::Ruler);
    _flagModeButton = createToolBarButton("Флажки", ":/resources/pin.png", MapMode::Flag);
    _kmlModeButton = createToolBarButton("Kml", ":/resources/kml.png", MapMode::Kml);

    _followButton = new QPushButton("Следовать", this);
    _followButton->setIcon(QIcon(":/resources/target.png"));
    _followButton->setCheckable(true);
    connect(_followButton, &QPushButton::toggled, this, [this](bool checked) {
        if (checked) {
            connect(_layers->deviceLayer().get(), &DeviceLayer::currentPositionChanged, this,
                    [this](const mcc::ui::core::GeoPosition& position) { centerOn(position.latLon()); });
            centerOnCurrent();
        } else {
            disconnect(_layers->deviceLayer().get(), &DeviceLayer::currentPositionChanged, this, 0);
        }
    });

    _modeButtons = {_rulerModeButton, _flagModeButton, _kmlModeButton};

    uncheckToolButtons();
    setType(_staticMapType);

    core::MapMode mode = core::Settings::instance()->mapMode();
    QAction* modeAction;
    switch(mode) {
    case core::MapMode::Online:
        modeAction = _onlineAction;
        break;
    case core::MapMode::Offline:
        modeAction = _offlineAction;
        break;
    default:
        modeAction = _stackAction;
    }
    modeAction->trigger();
}

void MapWidget::onOnlineModeChanged(const QString& buttonName, bool enableDownload, bool enableLayerButton,
                                    bool enableStackView)
{
    _onlineButton->setText(buttonName);
    _layers->mapLayer()->enableTileDownloading(enableDownload);
    _layerButton->setEnabled(enableLayerButton);
    _cacheStackView->setEnabled(enableStackView);
    if (enableStackView) {
        const StackCachePtr& cache = _cacheStackModel->stack();
        onCacheChanged(cache);
    } else {
        const OnlineCachePtr& cache = _cacheStackModel->onlineCache(_staticMapType);
        onCacheChanged(cache);
    }
    updateMap();
    adjustChildren();
}

QPushButton* MapWidget::createToolBarButton(const QString& text, const QString& iconPath, MapMode mode)
{
    QPushButton* button = new QPushButton(this);
    button->setText(text);
    button->setCheckable(true);
    button->setChecked(false);
    // button->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    button->setIcon(QIcon(iconPath));

    connect(button, &QPushButton::clicked, this, [this, button, mode]() {
        bool isChecked = button->isChecked();
        uncheckToolButtons();
        misc::Option<core::GeoBox> geoBox;
        if (!isChecked) {
            geoBox = _layers->setMode(MapMode::Default);
            emit modeChanged(MapMode::Default);
        } else {
            button->setChecked(true);
            geoBox = _layers->setMode(mode);
            emit modeChanged(mode);
        }
        if (geoBox.isSome()) {
            centerOn(geoBox.unwrap());
        }
        updateMap();
    });
    return button;
}

void MapWidget::uncheckToolButtons()
{
    for (QAbstractButton* b : _modeButtons) {
        b->setChecked(false);
    }
}

void MapWidget::onCacheChanged(const FileCachePtr& cache)
{
    MercatorProjection p1 = _rect->projection();
    _layers->mapLayer()->setMapInfo(cache);
    MercatorProjection p2 = cache->projection();
    _rect->setProjection(p2);
    _layers->changeProjection(p1, p2);
}

MapWidget::~MapWidget()
{
    delete _contextMenu;
    delete _cacheStackModel;
    delete _onlineMenu;
}

CacheStackView* MapWidget::cacheStackView()
{
    return _cacheStackView;
}

void MapWidget::paintEvent(QPaintEvent* event)
{
    (void)event;
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    _layers->draw(&p);
}

void MapWidget::resizeEvent(QResizeEvent* event)
{
    const QSize& newSize = event->size();
    int minZoom = minimumAllowedZoom(newSize);
    _slider->setMinimum(minZoom);
    int zoom = _rect->zoomLevel();
    if (minZoom > zoom) {
        _rect->setZoomLevel(minZoom);
        _layers->zoomEvent(_rect->visibleMapRect().center().toPoint(), zoom, minZoom);
    }
    _rect->resize(newSize.width(), newSize.height());
    event->accept();
    adjustChildren(newSize);

    // TODO: reset
    _layers->viewportResizeEvent(event->oldSize(), event->size());
    updateMap();
}

void MapWidget::adjustChildren(const QSize& size)
{
    _layerButton->adjustSize();
    _onlineButton->adjustSize();
    _flagModeButton->adjustSize();
    _rulerModeButton->adjustSize();
    _kmlModeButton->adjustSize();
    _followButton->adjustSize();

    _layerButton->move(size.width() - _layerButton->width() - subWidgetMargin, subWidgetMargin);
    _onlineButton->move(size.width() - _layerButton->width() - _onlineButton->width() - subWidgetMargin,
                        subWidgetMargin);

    QPoint modePos(subWidgetMargin, subWidgetMargin);

    _flagModeButton->move(modePos);
    _rulerModeButton->move(modePos.x() + _flagModeButton->width(), subWidgetMargin);
    _kmlModeButton->move(modePos.x() + _flagModeButton->width() + _rulerModeButton->width(), subWidgetMargin);

    _slider->move(subWidgetMargin, subWidgetMargin + _flagModeButton->pos().y() + _flagModeButton->height());
    _followButton->move(subWidgetMargin, size.height() - subWidgetMargin - _followButton->height());

    int minWidth = 3 * subWidgetMargin + _kmlModeButton->width() + _rulerModeButton->width() + _flagModeButton->width()
        + _onlineButton->width() + _layerButton->width();
    int minHeight = 4 * subWidgetMargin + _kmlModeButton->height() + _slider->height() + _followButton->height();
    setMinimumSize(minWidth, minHeight);
}

void MapWidget::adjustChildren()
{
    adjustChildren(size());
}

void MapWidget::mousePressEvent(QMouseEvent* event)
{
    mouseMoveEvent(event);
    _animator->stop();
    QPoint pos = event->pos();
    QPoint mapOffset = _rect->mapOffset(pos.x(), pos.y());
    _rect->setCursorPosition(mapOffset);
    setFocus(Qt::MouseFocusReason);
    Qt::MouseButton button = event->button();
    if (button == Qt::LeftButton) {
        event->accept();
        if (button & Qt::MiddleButton) {
            return;
        }
        _currentMousePos = pos;
        if (!_layers->mousePressEvent(mapOffset)) {
            _isMovingViewport = true;
        } else {
            updateMap();
        }
    } else if (button == Qt::MiddleButton) {
        event->accept();
        if (button & Qt::LeftButton) {
            return;
        }
        _isMovingViewport = true;
    }
}

void MapWidget::mouseReleaseEvent(QMouseEvent* event)
{
    QPoint pos = event->pos();
    QPoint mapOffset = _rect->mapOffset(pos.x(), pos.y());
    _rect->setCursorPosition(mapOffset);
    if (_isMovingViewport && core::Settings::instance()->mapAnimation()) {
        _animator->start();
    }
    if (event->button() == Qt::LeftButton) {
        _isMovingViewport = false;
        _layers->mouseReleaseEvent(mapOffset);
        updateMap();
    } else if (event->button() == Qt::MiddleButton) {
        _isMovingViewport = false;
    }
}

void MapWidget::scroll(const QPoint& delta)
{
    QPoint mapOffset = _rect->mapOffsetRaw();
    _rect->setCursorPosition(_rect->cursorPosition().unwrap() - delta);
    QPoint newMapOffset = mapOffset + delta;
    _rect->scroll(delta.x(), delta.y());
    _layers->viewportScrollEvent(mapOffset, newMapOffset);
    updateMap();
}

void MapWidget::mouseMoveEvent(QMouseEvent* event)
{
    QPoint pos = event->pos();
    QPoint delta = pos - _currentMousePos;
    QPoint mapOffset = _rect->mapOffsetRaw();
    _rect->setCursorPosition(mapOffset + pos);
    QPoint newMapOffset = mapOffset + delta;
    if (_isMovingViewport) {
        disconnect(_layers->deviceLayer().get(), &DeviceLayer::currentPositionChanged, this, 0);
        _followButton->setChecked(false);
        _rect->scroll(delta.x(), delta.y());
        _layers->viewportScrollEvent(mapOffset, newMapOffset);
    } else {
        _layers->mouseMoveEvent(mapOffset + pos, newMapOffset + pos);
    }
    _currentMousePos = pos;
    updateMap();
}

void MapWidget::mouseDoubleClickEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton) {
        zoom(event->pos(), 1);
    } else if (event->button() == Qt::RightButton) {
        zoom(event->pos(), -1);
    }
    event->accept();
}

void MapWidget::keyPressEvent(QKeyEvent* event)
{
    _rect->setModifiers(event->modifiers());
    int angle = 0;
    if (event->key() == Qt::Key_Plus) {
        angle = 1;
    } else if (event->key() == Qt::Key_Minus) {
        angle = -1;
    } else {
        QWidget::keyPressEvent(event);
        return;
    }
    setZoomLevel(angle + _rect->zoomLevel());
}

void MapWidget::keyReleaseEvent(QKeyEvent* event)
{
    _rect->setModifiers(event->modifiers());
    QWidget::keyReleaseEvent(event);
}

double MapWidget::minimumAllowedZoom() const
{
    return minimumAllowedZoom(size());
}

double MapWidget::minimumAllowedZoom(const QSize& size) const
{
    double zoom = std::log2(double(size.height()) / WebMapProperties::tilePixelSize()) + 1;
    return std::ceil(zoom);
}

void MapWidget::setZoomLevel(int zoomLevel)
{
    QPoint p(width() / 2, height() / 2);
    zoom(p, zoomLevel - _rect->zoomLevel());
}

void MapWidget::zoom(const QPoint& pos, int angle)
{
    int oldZoom = _rect->zoomLevel();
    int newZoom = angle + oldZoom;
    _slider->setValue(newZoom);
    if (newZoom > (int)WebMapProperties::maxZoom() || newZoom < minimumAllowedZoom()) {
        return;
    }
    QPoint mapOffset = _rect->mapOffset(pos.x(), pos.y());
    _rect->setCursorPosition(mapOffset);
    _rect->zoom(pos, angle);
    _layers->zoomEvent(mapOffset, oldZoom, newZoom);
    if (_followButton->isChecked()) {
        centerOnCurrent();
    }
    updateMap();
}

void MapWidget::centerOnCurrent()
{
    misc::Option<core::LatLon> pos = _layers->deviceLayer()->currentPosition();
    if (pos.isSome()) {
        centerOn(pos.unwrap());
    }
}

void MapWidget::wheelEvent(QWheelEvent* event)
{
    setFocus(Qt::MouseFocusReason);
    int angle = event->angleDelta().y();
    if (angle == 0) {
        return;
    }
    angle = angle / std::abs(angle);
    zoom(event->pos(), angle);
}

void MapWidget::contextMenuEvent(QContextMenuEvent* event)
{
    QPoint pos = event->pos();
    QPoint mapOffset = _rect->mapOffsetFull(pos.x(), pos.y());
    _layers->contextMenuEvent(mapOffset);
}

void MapWidget::centerOn(double lat, double lon)
{
    centerOn(lat, lon, _rect->zoomLevel());
}

void MapWidget::centerOn(double lat, double lon, int zoomLevel)
{
    _animator->stop();
    if (zoomLevel < minimumAllowedZoom()) {
        zoomLevel = minimumAllowedZoom();
    }
    if (zoomLevel > (int)WebMapProperties::maxZoom()) {
        zoomLevel = WebMapProperties::maxZoom();
    }
    int oldZoom = _rect->zoomLevel();
    QRect oldViewport = _rect->visibleMapRect().toRect();
    _rect->centerOn(lat, lon, zoomLevel);
    int newZoom = _rect->zoomLevel();
    _slider->setValue(newZoom);
    QRect newViewport = _rect->visibleMapRect().toRect();
    _layers->viewportResetEvent(oldZoom, newZoom, oldViewport, newViewport);
    updateMap();
}

void MapWidget::centerOn(const core::GeoBox& bbox)
{
    _animator->stop();
    int oldZoom = _rect->zoomLevel();
    QRect oldViewport = _rect->visibleMapRect().toRect();
    _rect->centerOn(bbox);
    int newZoom = _rect->zoomLevel();
    _slider->setValue(newZoom);
    QRect newViewport = _rect->visibleMapRect().toRect();
    _layers->viewportResetEvent(oldZoom, newZoom, oldViewport, newViewport);
    updateMap();
}

void MapWidget::centerOnDevice(const mcc::ui::core::FlyingDevice* device)
{
    const core::GeoPosition& pos = device->position();
    centerOn(pos.latitude, pos.longitude);
}

void MapWidget::centerOn(const core::LatLon& latLon)
{
    centerOn(latLon.latitude, latLon.longitude);
}

void MapWidget::setMode(MapMode mode)
{
    uncheckToolButtons();
    if (mode == MapMode::Ruler) {
        _rulerModeButton->toggle();
    } else if (mode == MapMode::Kml) {
        _kmlModeButton->toggle();
    } else if (mode == MapMode::Flag) {
        _flagModeButton->toggle();
    }
    misc::Option<core::GeoBox> geoBox = _layers->setMode(mode);
    if (geoBox.isSome()) {
        if (geoBox.unwrap().isPoint()) {
            centerOn(geoBox.unwrap().topLeft);
        } else {
            centerOn(geoBox.unwrap());
        }
    }
    emit modeChanged(mode);
    updateMap();
}

void MapWidget::setHomePosition(const core::LatLon& position)
{
    _layers->homeLayer()->setPosition(position);
}

void MapWidget::setHomePixmap(const QPixmap& pixmap)
{
    _layers->homeLayer()->setPixmap(pixmap);
}
}
}
}

