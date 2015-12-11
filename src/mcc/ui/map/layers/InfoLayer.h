/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#pragma once

#include "mcc/ui/map/layers/Layer.h"

#include <QString>
#include <QSize>

#include <cstdint>

class QPainter;

namespace mcc {
namespace ui {
namespace map {

class InfoLayer : public Layer {
public:
    InfoLayer(const MapRectConstPtr& rect);

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

    void setMargins(int x, int y);

private:
    void update();
    void setPixelDensity(double density);
    QString _text;
    int _xMargin;
    int _yMargin;
    QSize _size;
    int _rulerWidthPixels;
    double _pixelDensity;
};
}
}
}
