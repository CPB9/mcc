/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#pragma once

#include "mcc/ui/map/layers/SimpleFlagLayer.h"
#include "mcc/ui/map/drawables/RulerPolyLine.h"
#include "mcc/ui/map/drawables/BiMarker.h"

class QMenu;
class QAction;

namespace mcc {
namespace ui {
namespace core {
class Route;
class FlyingDevice;
}
namespace map {

class WaypointLayer : public SimpleFlagLayer {
    Q_OBJECT
public:
    enum Style { Inactive, Active, Editable };
    WaypointLayer(mcc::ui::core::FlyingDevice* device, mcc::ui::core::Route* route, const MapRectConstPtr& rect);
    ~WaypointLayer() override;
    void draw(QPainter* p) const override;
    bool viewportResetEvent(int oldZoom, int newZoom, const QRect& oldViewpiort, const QRect& newViewport) override;
    bool zoomEvent(const QPoint& pos, int fromZoom, int toZoom) override;
    void changeProjection(const MercatorProjection& from, const MercatorProjection& to) override;

    void setStyle(Style style);

protected:
    void addFlagAt(const QPointF& pos) override;
    void insertFlagAt(std::size_t index, const QPointF& pos) override;
    misc::Option<std::size_t> flagAt(const QPointF& pos) override;
    misc::Option<LineIndexAndPos> lineAt(const QPointF& pos) override;
    void moveCurrentBy(const QPointF& delta) override;
    void setCurrentActive(bool isActive) override;
    bool showContextMenuForCurrent(const QPoint& pos) override;
    bool showContextMenuForNone(const QPoint& pos) override;
    void showFlagEditor(const QPoint& pos) override;
    void finishMovingCurrent() override;

private:
    void removeActive();
    void updateMarker(std::size_t index);
    void updateMarkerPixmap(BiMarker* marker, std::size_t index, core::WaypointFlags flags) const;
    QPixmap createPixmap(std::size_t index, bool active, core::WaypointFlags flags) const;
    void createPlaceholders();
    QImage createPlaceholderImage(bool active) const;
    BiMarker createMarker(const core::Waypoint& waypoint, std::size_t index) const;
    BiMarker createMarker(const QPointF& pos, std::size_t index, core::WaypointFlags flags = 0) const;
    void repaintMarkerPixmapsFrom(std::size_t start);
    void resetWaypoints();
    mcc::ui::core::FlyingDevice* _device;
    mcc::ui::core::Route* _model;
    RulerPolyLine<BiMarker> _markers;
    Style _style;
    bool _catchUpdates;
    QMenu* _contextMenu;
    QAction* _removeAction;
    QAction* _copyAction;
    QPixmap _emptyPixmap;
    std::size_t _lastTargetWaypoint;
};
}
}
}
