/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#pragma once

#include "mcc/ui/core/Structs.h"
#include "mcc/ui/map/Ptr.h"
#include "mcc/ui/map/StaticMapType.h"
#include "mcc/ui/map/MapMode.h"

#ifdef MCC_USE_OPENGL
#define MCC_MAP_WIDGET_BASE QGLWidget
#include <QGLWidget>
#else
#define MCC_MAP_WIDGET_BASE QWidget
#include <QWidget>
#endif

#include <vector>
#include <map>

class QMenu;
class QActionGroup;
class QButtonGroup;
class QPushButton;
class QAction;
class QLineEdit;
class QToolBar;
class QDialog;
class QToolButton;

namespace mcc {
namespace ui {

namespace core {
class DeviceManager;
class FlyingDevice;
}
namespace map {

class Layer;
class MapSlider;
class MapLayer;
class InfoLayer;
class RulerLayer;
class RulerLayer2;
class DeviceLayer;
class KmlLayer;
class FlagLayer;
class GridLayer;
class KmlModel;
class KmlModelLayer;
class CacheStackView;
class CacheStackModel;
class MapWidgetAnimator;


class MapWidget : public MCC_MAP_WIDGET_BASE {
    Q_OBJECT
public:
    MapWidget(mcc::ui::core::DeviceManager* manager, QWidget* parent = 0);
    ~MapWidget() override;

    void paintEvent(QPaintEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void wheelEvent(QWheelEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;
    void keyReleaseEvent(QKeyEvent*) override;
    void contextMenuEvent(QContextMenuEvent* event) override;
    void mouseDoubleClickEvent(QMouseEvent*) override;

    void scroll(const QPoint& delta);

    KmlModel* kmlModel() const;

    CacheStackView* cacheStackView();
signals:
    void modeChanged(MapMode mode);
    void mapNeedsUpdate();

public slots:
    void setMode(MapMode mode);
    void setHomePosition(const core::LatLon& position);
    void setHomePixmap(const QPixmap& pixmap);

    void setZoomLevel(int zoomLevel);
    void zoom(const QPoint& pos, int angle);
    void centerOn(double lat, double lon);
    void centerOn(const core::LatLon& latLon);
    void centerOn(double lat, double lon, int zoomLevel);
    void centerOn(const core::GeoBox& bbox);
    void centerOnCurrent();
    void centerOnDevice(const mcc::ui::core::FlyingDevice* device);
    const MapRectConstPtr& mapRect() const;

private slots:
    void updateMap();

private:
    void createMapWidgets();
    void createStack();
    void createLayers(core::DeviceManager* manager, KmlModel* kmlModel);
    void uncheckToolButtons();
    QPushButton* createToolBarButton(const QString& text, const QString& iconPath, MapMode mode);
    void onCacheChanged(const FileCachePtr& cache);
    void onOnlineModeChanged(const QString& buttonName, bool enableDownload, bool enableLayerButton, bool enableStackView);
    void adjustChildren();
    void adjustChildren(const QSize& size);
    double minimumAllowedZoom(const QSize& size) const;
    double minimumAllowedZoom() const;
    MapSlider* _slider;

    MapRectPtr _rect;
    MapRectConstPtr _constRect;
    LayerGroupPtr _layers;

    std::map<StaticMapType, QAction*> _typeToAction;
    std::vector<QPushButton*> _modeButtons;

    CacheStackModel* _cacheStackModel;
    CacheStackView* _cacheStackView;

    QActionGroup* _layerActions;
    QAction* _resetRulerAction;
    QPushButton* _layerButton;
    QPushButton* _onlineButton;

    QMenu* _contextMenu;
    QMenu* _layerMenu;


    QAction* _onlineAction;
    QAction* _offlineAction;
    QAction* _stackAction;
    QActionGroup* _onlineActions;
    QMenu* _onlineMenu;

    QPushButton* _flagModeButton;
    QPushButton* _rulerModeButton;
    QPushButton* _kmlModeButton;
    QPushButton* _followButton;

    KmlModel* _kmlModel;

    std::unique_ptr<MapWidgetAnimator> _animator;

    QPoint _currentMousePos;

    bool _isMovingViewport;
    bool _wasOnline;
    StaticMapType _staticMapType;
};

inline const MapRectConstPtr& MapWidget::mapRect() const
{
    return _constRect;
}
}
}
}
