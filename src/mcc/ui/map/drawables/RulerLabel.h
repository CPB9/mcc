/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#pragma once

#include "mcc/ui/map/drawables/Interfaces.h"
#include "mcc/ui/map/drawables/Point.h"

namespace mcc {
namespace ui {
namespace map {

class RulerLabel : public AbstractMarker<RulerLabel> {
public:
    RulerLabel(const QPointF& position, double distance, double rotataion);

    void draw(QPainter* p, const MapRectConstPtr& rect) const;
    QRectF rect() const;
    void moveBy(const QPointF& delta);
    void changeZoomLevel(int from, int to);

    const QPointF& position() const;
    void setPosition(const QPointF& position);

    void setVisible(bool isVisible);

    void setDistanceAndRotation(double distance, double rotation);

private:
    double _rotation;
    QPixmap _pixmap;
    Point _point;
    QPointF _offset;
    bool _isVisible;
};

inline const QPointF& RulerLabel::position() const
{
    return _point.position();
}

inline void RulerLabel::setVisible(bool isVisible)
{
    _isVisible = isVisible;
}


inline void RulerLabel::setPosition(const QPointF& position)
{
    _point.setPosition(position);
}

inline void RulerLabel::changeZoomLevel(int from, int to)
{
    _point.changeZoomLevel(from, to);
}

inline RulerLabel::RulerLabel(const QPointF& position, double distance, double rotataion)
    : _point(position)
    , _isVisible(true)
{
    setDistanceAndRotation(distance, rotataion);
}

inline void RulerLabel::moveBy(const QPointF& delta)
{
    _point.moveBy(delta);
}

inline QRectF RulerLabel::rect() const
{
    return WithRect::positionedRect(_pixmap.rect(), _point.position());
}
}
}
}
