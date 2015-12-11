/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#pragma once

#include "mcc/misc/Option.h"

#include <cstddef>

class QPointF;

namespace mcc {
namespace ui {
namespace map {

class WithPoints {
public:
    virtual misc::Option<std::size_t> nearestPoint() const = 0;
    virtual void movePointBy(std::size_t index, const QPointF& delta) = 0;
    virtual void appendPoint(const QPointF& point) = 0;
    virtual void insertPoint(std::size_t index, const QPointF& point) = 0;
    virtual void removePoint(std::size_t index) = 0;
    virtual std::size_t pointNum() const = 0;
};
}
}
}
