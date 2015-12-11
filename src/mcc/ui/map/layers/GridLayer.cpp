/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "mcc/ui/map/layers/GridLayer.h"
#include "mcc/ui/map/MapRect.h"

#include <QPainter>
#include <QDebug>

namespace mcc {
namespace ui {
namespace map {

static int numPoints[22] = {
    36, // z1
    36, // z2
    36, // z3
    36, // z4
    36, // z5
    36, // z6
    36, // z7
    72, // z8
    180, // z9
    360, // z10
    720, // z11
    1080, // z12
    2160, // z13
    4320, // z14
    10800, // z15
    21600, // z16
    43200, // z17
    64800, // z18
    129600, // z19
    259200, // z20
    648000, // z21
    1296000, // z22
};

GridLayer::GridLayer(const MapRectConstPtr& rect)
    : Layer(rect)
{
    updateLines();
}

GridLayer::~GridLayer()
{
}

void GridLayer::draw(QPainter* p) const
{
    QPen pen;
    pen.setColor(Qt::gray);
    pen.setWidthF(0.5);
    p->setPen(pen);
    QPointF start;
    start.setY(0);
    QPointF end;
    end.setY(mapRect()->size().height());
    for (double x : _vlineStarts) {
        start.setX(x);
        end.setX(x);
        p->drawLine(start, end);
    }
}

void GridLayer::updateLines()
{
    _vlineStarts.clear();
    double n = numPoints[mapRect()->zoomLevel()];
    double degreeDelta = 360.0 / n;
    double lastLon = std::fmod(mapRect()->lon(mapRect()->size().width()), 360);
    double nearestLon = mapRect()->lon();
    nearestLon = std::ceil(nearestLon / degreeDelta) * degreeDelta;
    double rx;
    double mapOffsetX = mapRect()->mapOffsetX();
    do {
        rx = mapRect()->projection().longitudeToRelativeOffset(nearestLon);
        double xStart = mapRect()->fromRelativeOffsetX(rx);
        nearestLon = std::fmod(nearestLon + degreeDelta, 360);
        _vlineStarts.push_back(xStart - mapOffsetX);
    } while (nearestLon < lastLon);
}

bool GridLayer::mousePressEvent(const QPoint& pos)
{
    (void)pos;
    return false;
}

bool GridLayer::mouseReleaseEvent(const QPoint& pos)
{
    (void)pos;
    return false;
}

bool GridLayer::mouseMoveEvent(const QPoint& oldPos, const QPoint& newPos)
{
    (void)oldPos;
    (void)newPos;
    return false;
}

bool GridLayer::viewportResizeEvent(const QSize& oldSize, const QSize& newSize)
{
    (void)oldSize;
    (void)newSize;
    updateLines();
    return true;
}

bool GridLayer::viewportScrollEvent(const QPoint& oldPos, const QPoint& newPos)
{
    (void)oldPos;
    (void)newPos;
    updateLines();
    return true;
}

bool GridLayer::contextMenuEvent(const QPoint& pos)
{
    (void)pos;
    return false;
}

bool GridLayer::viewportResetEvent(int oldZoom, int newZoom, const QRect& oldViewpiort, const QRect& newViewport)
{
    (void)oldZoom;
    (void)newZoom;
    (void)oldViewpiort;
    (void)newViewport;
    updateLines();
    return true;
}

void GridLayer::changeProjection(const MercatorProjection& from, const MercatorProjection& to)
{
    (void)from;
    (void)to;
    updateLines();
}

bool GridLayer::zoomEvent(const QPoint& pos, int fromZoom, int toZoom)
{
    (void)pos;
    (void)fromZoom;
    (void)toZoom;
    updateLines();
    return true;
}
}
}
}
