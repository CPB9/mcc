/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#pragma once

#include "mcc/ui/map/drawables/Interfaces.h"
#include "mcc/ui/map/MapRect.h"

#include <QPointF>

class QPainter;
class QSizeF;

namespace mcc {
namespace ui {
namespace core {
struct LatLon;
}
namespace map {

class Point : public WithPosition<Point>, public Zoomable<Point>, public Movable<Point> {
public:
    Point(const QPointF& position);
    Point(double x = 0, double y = 0);

    void moveBy(const QPointF& delta);
    void changeZoomLevel(int from, int to);

    void setPosition(const QPointF& point);
    const QPointF& position() const;

private:
    QPointF _position;
};

inline Point::Point(const QPointF& position)
    : _position(position)
{
}

inline Point::Point(double x, double y)
    : _position(x, y)
{
}

inline void Point::setPosition(const QPointF& point)
{
    _position = point;
}

inline const QPointF& Point::position() const
{
    return _position;
}

inline void Point::moveBy(const QPointF& delta)
{
    setPosition(_position + delta);
}

inline void Point::changeZoomLevel(int from, int to)
{
    double ratio = Zoomable::calcZoomRatio(from, to);
    setPosition(_position * ratio);
}
}
}
}
