/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "mcc/ui/map/layers/InfoLayer.h"
#include "mcc/ui/map/MapRect.h"

#include <QPainter>
#include <QDebug>

#include <cmath>
#include <cassert>

namespace mcc {
namespace ui {
namespace map {

static const int maxRulerWidth = 200;

InfoLayer::InfoLayer(const MapRectConstPtr& rect)
    : Layer(rect)
    , _xMargin(30)
    , _yMargin(30)
    , _size(0, 0)
    , _pixelDensity(1)
{
}

void InfoLayer::draw(QPainter* p) const
{
    p->setBrush(Qt::yellow);
    p->setPen(Qt::black);
    int rulerHeight = 10;
    QTransform t = p->transform();
    p->translate(_size.width() - _xMargin - _rulerWidthPixels, _size.height() - _yMargin - rulerHeight);
    p->fillRect(0, 0, _rulerWidthPixels, rulerHeight, Qt::yellow);
    p->drawRect(0, 0, _rulerWidthPixels, rulerHeight);
    auto rect = p->fontMetrics().boundingRect(_text);
    QPoint textOffset(_rulerWidthPixels - rect.width() - rect.x(), -rect.height() - rect.y());
    p->drawText(textOffset, _text);
    p->setTransform(t);
}

void InfoLayer::changeProjection(const MercatorProjection& from, const MercatorProjection& to)
{
    (void)from;
    (void)to;
    update();
}

bool InfoLayer::contextMenuEvent(const QPoint& pos)
{
    (void)pos;
    return false;
}

bool InfoLayer::mouseMoveEvent(const QPoint& oldPos, const QPoint& newPos)
{
    (void)oldPos;
    (void)newPos;
    return false;
}

bool InfoLayer::mousePressEvent(const QPoint& pos)
{
    (void)pos;
    return false;
}

bool InfoLayer::mouseReleaseEvent(const QPoint& pos)
{
    (void)pos;
    return false;
}

bool InfoLayer::viewportResetEvent(int oldZoom, int newZoom, const QRect& oldViewpiort, const QRect& newViewport)
{
    (void)newZoom;
    (void)oldZoom;
    (void)oldViewpiort;
    _size = newViewport.size();
    update();
    return true;
}

bool InfoLayer::viewportResizeEvent(const QSize& oldSize, const QSize& newSize)
{
    (void)oldSize;
    _size = newSize;
    update();
    return true;
}

bool InfoLayer::viewportScrollEvent(const QPoint& oldPos, const QPoint& newPos)
{
    (void)oldPos;
    (void)newPos;
    update();
    return true;
}

bool InfoLayer::zoomEvent(const QPoint& pos, int fromZoom, int toZoom)
{
    (void)pos;
    (void)fromZoom;
    (void)toZoom;
    update();
    return false;
}

void InfoLayer::update()
{
    const MapRect* rect = mapRect().get();
    setPixelDensity(rect->horisontalPixelLength(rect->size().height() / 2));
}

static inline uint64_t pow10Positive(uint exp)
{
    static const uint64_t values[20]
        = {1u, 10u, 100u, 1000u, 10000u, 100000u, 1000000u, 10000000u, 100000000u, 1000000000u, 10000000000u,
           100000000000u, 1000000000000u, 10000000000000u, 100000000000000u, 1000000000000000u, 10000000000000000u,
           100000000000000000u, 1000000000000000000u, 10000000000000000000u};
    assert(exp < 20);
    return values[exp];
}

void InfoLayer::setPixelDensity(double density)
{
    _pixelDensity = density;
    double maxRulerWidthM = _pixelDensity * maxRulerWidth;
    uint64_t rulerWidthM = maxRulerWidthM;
    uint64_t divisor = pow10Positive(std::floor(std::log10(rulerWidthM)));
    uint64_t adjustedRulerWidthM = rulerWidthM / divisor * divisor;
    _rulerWidthPixels = maxRulerWidth * adjustedRulerWidthM / maxRulerWidthM;
    if (rulerWidthM > 1000) {
        _text.setNum(adjustedRulerWidthM / 1000);
        _text.append(" км");
    } else {
        _text.setNum(adjustedRulerWidthM);
        _text.append(" м");
    }
}

void InfoLayer::setMargins(int x, int y)
{
    _xMargin = x;
    _yMargin = y;
}
}
}
}
