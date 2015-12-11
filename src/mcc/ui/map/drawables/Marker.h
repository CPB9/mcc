/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#pragma once

#include "mcc/ui/map/drawables/MarkerBase.h"
#include "mcc/ui/map/drawables/Interfaces.h"
#include "mcc/ui/map/drawables/Point.h"

#include <QPixmap>
#include <QPainter>

class QPainter;

namespace mcc {
namespace ui {
namespace map {

class Marker : public MarkerBase, public AbstractMarker<Marker> {
public:
    Marker(const QPointF& position = QPointF(0, 0), Qt::Alignment alignment = Qt::AlignCenter);

    void draw(QPainter* p, const MapRectConstPtr& rect) const;
    QRectF rect() const;

    void moveBy(const QPointF& delta);
    void changeZoomLevel(int from, int to);
    const QPointF& position() const;
    void setPosition(const QPointF& position);

    QPointF alignedPosition() const;

private:
    Point _point;
};

inline Marker::Marker(const QPointF& position, Qt::Alignment alignment)
    : MarkerBase(alignment)
    , _point(position)
{
}

inline void Marker::setPosition(const QPointF& position)
{
    _point.setPosition(position);
}

inline const QPointF& Marker::position() const
{
    return _point.position();
}

inline void Marker::changeZoomLevel(int from, int to)
{
    _point.changeZoomLevel(from, to);
}

inline void Marker::draw(QPainter* p, const MapRectConstPtr& rect) const
{
    drawMarker(p, position() - rect->mapOffsetRaw());
}

inline void Marker::moveBy(const QPointF& delta)
{
    _point.moveBy(delta);
}

inline QRectF Marker::rect() const
{
    return WithRect<Marker>::positionedRect(pixmap().rect(), _point.position() + offset());
}

inline QPointF Marker::alignedPosition() const
{
    return _point.position() + offset();
}
}
}
}
