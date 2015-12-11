/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#pragma once

class QPointF;

namespace mcc {
namespace ui {
namespace map {

template <typename B>
class Movable {
public:
    void moveBy(const QPointF& delta);
};

template <typename B>
inline void Movable<B>::moveBy(const QPointF& delta)
{
    return static_cast<B*>(this)->moveBy(delta);
}
}
}
}
