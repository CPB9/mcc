/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#pragma once

#include "mcc/ui/map/layers/Layer.h"
#include "mcc/ui/map/drawables/PolyLine.h"
#include "mcc/ui/map/drawables/Point.h"
#include "mcc/ui/map/drawables/RulerLabel.h"
#include "mcc/ui/map/drawables/Marker.h"

#include <array>

namespace mcc {
namespace ui {
namespace core {
class Route;
class FlyingDevice;
}
namespace map {

class WaypointLayer;

class TemplateLayer : public Layer {
    Q_OBJECT
public:
    TemplateLayer(const MapRectConstPtr& rect);
    ~TemplateLayer() override;
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
    WaypointLayer* intoWaypointLayer(mcc::ui::core::FlyingDevice* device, mcc::ui::core::Route* route, double distance, double height, double speed) const;

private:
    enum State { None, Moving, Rotating, Scaling };
    State computeState(const QPoint& pos) const;
    void rotate(QPointF* point, double angle) const;
    QPointF rotated(const QPointF& point) const;
    bool nearby(const QPointF& first, const QPointF& second) const;
    bool updateCursor(State state);
    std::vector<core::LatLon> createScanLine(const QPointF& p1, const QPointF& p2, double distance) const;

    State _state;
    QPixmap _rotatePixmap;
    QPixmap _expandPixmap;
    QPointF _center;
    QPointF _pressedPos;
    double _width;
    double _height;
    double _rotation;
    double _oldWidth;
    double _oldHeight;
    double _oldRotation;
};
}
}
}
