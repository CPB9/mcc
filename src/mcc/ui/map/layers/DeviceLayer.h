/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#pragma once

#include "mcc/ui/map/layers/Layer.h"
#include "mcc/ui/core/Structs.h"
#include "mcc/misc/Option.h"

#include <vector>

#include <QPointF>
#include <QColor>
#include <QObject>

class QPainter;
class QMenu;
class QAction;

namespace mcc {
namespace ui {
namespace core {
class Route;
class DeviceManager;
class FlyingDevice;
}
namespace map {

class MapRect;
class AircraftLayer;
class WaypointLayer;
class AircraftWaypointsPair;
class DeviceLayer;

class DeviceLayer : public Layer {
    Q_OBJECT
public:
    DeviceLayer(mcc::ui::core::DeviceManager* manager, const MapRectConstPtr& rect);
    ~DeviceLayer() override;
    void draw(QPainter* p) const override;
    bool mouseMoveEvent(const QPoint& oldPos, const QPoint& newPos) override;
    bool mousePressEvent(const QPoint& pos) override;
    bool mouseReleaseEvent(const QPoint& pos) override;
    bool viewportResizeEvent(const QSize& oldSize, const QSize& newSize) override;
    bool viewportScrollEvent(const QPoint& oldPos, const QPoint& newPos) override;
    bool contextMenuEvent(const QPoint& pos) override;
    bool zoomEvent(const QPoint& pos, int fromZoom, int toZoom) override;
    void changeProjection(const MercatorProjection& from, const MercatorProjection& to) override;
    bool viewportResetEvent(int oldZoom, int newZoom, const QRect& oldViewpiort, const QRect& newViewport) override;

    void setEditableLayerVisible(bool isVisible);
    misc::Option<core::GeoBox> currentBbox() const;
    misc::Option<core::LatLon> currentPosition() const;

signals:
    void currentPositionChanged(const core::GeoPosition& position);

private:
    bool trySelectDeviceAt(const QPoint& pos);
    AircraftWaypointsPair* activeDevice() const;
    void setCurrentActive(Layer* layer);
    void setCurrentActive(bool editTemplate = false);
    void setActiveWaypointStyle(AircraftWaypointsPair* pair);
    void setInactiveWaypointStyle(AircraftWaypointsPair* pair);
    void setCurrentDevice(core::FlyingDevice* device);
    void reorderLayers();
    template <typename F, typename... A>
    void visitLayers(F func, A&&... args);
    template <typename F, typename... A>
    bool doIfVisible(F func, A&&... args);
    mcc::ui::core::DeviceManager* _manager;
    std::vector<std::unique_ptr<AircraftWaypointsPair>> _layers;
    std::unique_ptr<Layer> _editableWaypointLayer;
    misc::Option<std::size_t> _activeDevice;
    bool _isEditableLayerVisible;
    QMenu* _menu;
    QAction* _resetRouteAction;
    QAction* _applyRouteAction;
    QAction* _clearRouteAction;
    QAction* _endEditRouteAction;
};
}
}
}
