/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#pragma once

class QPainter;

namespace mcc {
namespace ui {
namespace map {

template <typename B>
class Drawable {
public:
    void draw(QPainter* p) const;
};

template <typename B>
inline void Drawable<B>::draw(QPainter* p) const
{
    return static_cast<B*>(this)->draw(p);
}
}
}
}
