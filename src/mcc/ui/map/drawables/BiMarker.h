/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#pragma once

#include "mcc/ui/map/drawables/Interfaces.h"
#include "mcc/ui/map/drawables/Marker.h"
#include "mcc/ui/map/drawables/Point.h"

#include <QPixmap>
#include <QPainter>
#include <Qt>

class QPainter;

namespace mcc {
namespace ui {
namespace map {

class BiMarker : public AbstractMarker<BiMarker> {
public:
    BiMarker();
    BiMarker(const QPointF& position);

    void setActive(bool isActive);
    bool isActive() const;

    void setActiveAlignment(Qt::Alignment alignment);
    void setActivePixmap(const QPixmap& pixmap);
    const QPixmap& activePixmap() const;
    QRectF activeRect() const;

    void setInactiveAlignment(Qt::Alignment alignment);
    void setInactivePixmap(const QPixmap& pixmap);
    const QPixmap& inactivePixmap() const;
    QRectF inactiveRect() const;

    void draw(QPainter* p, const MapRectConstPtr& rect) const;
    QRectF rect() const;
    void moveBy(const QPointF& delta);
    void changeZoomLevel(int from, int to);

    void setPosition(const QPointF& position);
    const QPointF& position() const;

protected:
    MarkerBase _active;
    MarkerBase _inactive;

private:
    Point _point;
    bool _isActive;
};

inline BiMarker::BiMarker(const QPointF& position)
    : _point(position)
    , _isActive(false)
{
}

inline BiMarker::BiMarker()
    : _point(0, 0)
    , _isActive(false)
{
}

inline bool BiMarker::isActive() const
{
    return _isActive;
}

inline void BiMarker::setActive(bool isActive)
{
    _isActive = isActive;
}

inline const QPixmap& BiMarker::activePixmap() const
{
    return _active.pixmap();
}

inline QRectF BiMarker::activeRect() const
{
    return _active.rect();
}

inline const QPixmap& BiMarker::inactivePixmap() const
{
    return _inactive.pixmap();
}

inline QRectF BiMarker::inactiveRect() const
{
    return _inactive.rect();
}

inline void BiMarker::setActiveAlignment(Qt::Alignment alignment)
{
    _active.setAlignment(alignment);
}

inline void BiMarker::setActivePixmap(const QPixmap& pixmap)
{
    _active.setPixmap(pixmap);
}

inline void BiMarker::setInactiveAlignment(Qt::Alignment alignment)
{
    _inactive.setAlignment(alignment);
}

inline void BiMarker::setInactivePixmap(const QPixmap& pixmap)
{
    _inactive.setPixmap(pixmap);
}

inline const QPointF& BiMarker::position() const
{
    return _point.position();
}

inline void BiMarker::setPosition(const QPointF& position)
{
    _point.setPosition(position);
}

inline void BiMarker::changeZoomLevel(int from, int to)
{
    _point.changeZoomLevel(from, to);
}

inline void BiMarker::moveBy(const QPointF& delta)
{
    _point.moveBy(delta);
}

inline void BiMarker::draw(QPainter* p, const MapRectConstPtr& rect) const
{
    if (isActive()) {
        _active.drawMarker(p, _point.position() - rect->mapOffsetRaw());
    } else {
        _inactive.drawMarker(p, _point.position() - rect->mapOffsetRaw());
    }
}

inline QRectF BiMarker::rect() const
{
    if (isActive()) {
        return WithRect::positionedRect(_active.rect(), _point.position() + _active.offset());
    }
    return WithRect::positionedRect(_inactive.rect(), _point.position() + _inactive.offset());
}
}
}
}
