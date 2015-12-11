/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#pragma once

#include "mcc/ui/map/layers/Layer.h"
#include "mcc/ui/map/drawables/Flag.h"

namespace mcc {
namespace ui {
namespace map {

class HomeLayer : public Layer {
    Q_OBJECT
public:
    HomeLayer(const MapRectConstPtr& rect);
    ~HomeLayer() override;

    void draw(QPainter* p) const override;
    bool mousePressEvent(const QPoint& pos) override;
    bool mouseReleaseEvent(const QPoint& pos) override;
    bool mouseMoveEvent(const QPoint& oldPos, const QPoint& newPos) override;
    bool viewportResizeEvent(const QSize& oldSize, const QSize& newSize) override;
    bool viewportScrollEvent(const QPoint& oldPos, const QPoint& newPos) override;
    bool contextMenuEvent(const QPoint& pos) override;
    bool zoomEvent(const QPoint& pos, int fromZoom, int toZoom) override;
    bool viewportResetEvent(int oldZoom, int newZoom, const QRect& oldViewpiort, const QRect& newViewport) override;
    void changeProjection(const MercatorProjection& from, const MercatorProjection& to) override;

    void setPosition(const core::LatLon& latLon);
    void setPixmap(const QPixmap& pixmap);

private:
    Flag _flag;
};
}
}
}
