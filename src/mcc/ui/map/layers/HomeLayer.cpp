/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "mcc/ui/map/layers/HomeLayer.h"
#include "mcc/ui/core/GlobalCoordinatePrinter.h"

namespace mcc {
namespace ui {
namespace map {

HomeLayer::HomeLayer(const MapRectConstPtr& rect)
    : Layer(rect)
    , _flag(rect)
{
}

HomeLayer::~HomeLayer()
{
}

void HomeLayer::changeProjection(const MercatorProjection& from, const MercatorProjection& to)
{
    _flag.changeProjection(mapRect(), from, to);
    connect(core::GlobalCoordinatePrinter::instance(), &core::GlobalCoordinatePrinter::printerChanged, this, [this](){
        _flag.updateLabel();
    });
}

bool HomeLayer::contextMenuEvent(const QPoint& pos)
{
    (void)pos;
    return false;
}

void HomeLayer::draw(QPainter* p) const
{
    _flag.draw(p, mapRect());
}

bool HomeLayer::mouseMoveEvent(const QPoint& oldPos, const QPoint& newPos)
{
    (void)oldPos;
    (void)newPos;
    return false;
}

bool HomeLayer::mousePressEvent(const QPoint& pos)
{
    (void)pos;
    return false;
}

bool HomeLayer::mouseReleaseEvent(const QPoint& pos)
{
    (void)pos;
    return false;
}

bool HomeLayer::viewportResetEvent(int oldZoom, int newZoom, const QRect& oldViewpiort, const QRect& newViewport)
{
    (void)oldViewpiort;
    (void)newViewport;
    _flag.changeZoomLevel(oldZoom, newZoom);
    return true;
}

bool HomeLayer::viewportResizeEvent(const QSize& oldSize, const QSize& newSize)
{
    (void)oldSize;
    (void)newSize;
    return false;
}

bool HomeLayer::viewportScrollEvent(const QPoint& oldPos, const QPoint& newPos)
{
    (void)oldPos;
    (void)newPos;
    return false;
}

bool HomeLayer::zoomEvent(const QPoint& pos, int fromZoom, int toZoom)
{
    (void)pos;
    _flag.changeZoomLevel(fromZoom, toZoom);
    return true;
}

void HomeLayer::setPixmap(const QPixmap& pixmap)
{
    _flag.setActive(true);
    _flag.setActivePixmap(pixmap);
}

void HomeLayer::setPosition(const core::LatLon& latLon)
{
    _flag.setLatLon(latLon, mapRect());
}
}
}
}
