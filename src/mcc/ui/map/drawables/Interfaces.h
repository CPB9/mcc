/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#pragma once

#include "mcc/ui/map/drawables/Drawable.h"
#include "mcc/ui/map/drawables/Movable.h"
#include "mcc/ui/map/drawables/WithRect.h"
#include "mcc/ui/map/drawables/WithPosition.h"
#include "mcc/ui/map/drawables/WithPoints.h"
#include "mcc/ui/map/drawables/Zoomable.h"

namespace mcc {
namespace ui {
namespace map {

template <typename B>
class AbstractShape : public Drawable<B>, public WithRect<B>, public Movable<B>, public Zoomable<B> {
};

template <typename B>
class AbstractMarker : public AbstractShape<B>, public WithPosition<B> {
};

template <typename B>
class AbstractPolyLine : public AbstractShape<B> { //, public WithPoints<B> {
};
}
}
}
