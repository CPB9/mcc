/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#pragma once

#include <QRectF>

class QPointF;

namespace mcc {
namespace ui {
namespace map {

template <typename B = void>
class WithRect {
public:
    bool hasInRect(const QPointF& position) const;
    QRectF positionedRect(const QPointF& position);
    static QRectF positionedRect(const QRectF& rect, const QPointF& position);
};

template <typename B>
inline bool WithRect<B>::hasInRect(const QPointF& position) const
{
    return static_cast<const B*>(this)->rect().contains(position);
}

template <typename B>
inline QRectF WithRect<B>::positionedRect(const QRectF& rect, const QPointF& position)
{
    return QRectF(position, rect.size());
}

template <typename B>
inline QRectF WithRect<B>::positionedRect(const QPointF& position)
{
    return positionedRect(static_cast<B*>(this)->rect(), position);
}
}
}
}
