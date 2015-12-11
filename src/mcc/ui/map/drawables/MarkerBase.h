/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#pragma once

#include "mcc/ui/map/drawables/Point.h"

#include <QPixmap>
#include <QPainter>
#include <Qt>

class QPainter;

namespace mcc {
namespace ui {
namespace map {

class MarkerBase {
public:
    MarkerBase(Qt::Alignment alignment = Qt::AlignCenter);
    void drawMarker(QPainter* p, const QPointF& position) const;
    void setAlignment(Qt::Alignment alignment);
    void setPixmap(const QPixmap& pixmap);
    const QPointF& offset() const;
    const QPixmap& pixmap() const;
    QRectF rect() const;

private:
    void recalcOffset();
    Qt::Alignment _alignment;
    QPointF _offset;
    QPixmap _pixmap;
};

inline MarkerBase::MarkerBase(Qt::Alignment alignment)
    : _alignment(alignment)
{
    recalcOffset();
}

inline QRectF MarkerBase::rect() const
{
    return _pixmap.rect();
}

inline void MarkerBase::drawMarker(QPainter* p, const QPointF& position) const
{
    p->drawPixmap(position + _offset, _pixmap);
}

inline const QPointF& MarkerBase::offset() const
{
    return _offset;
}

inline void MarkerBase::recalcOffset()
{
    _offset = WithPosition<>::edgePoint(_pixmap.size(), _alignment);
}

inline const QPixmap& MarkerBase::pixmap() const
{
    return _pixmap;
}

inline void MarkerBase::setAlignment(Qt::Alignment alignment)
{
    _alignment = alignment;
    recalcOffset();
}

inline void MarkerBase::setPixmap(const QPixmap& pixmap)
{
    _pixmap = pixmap;
    recalcOffset();
}
}
}
}
