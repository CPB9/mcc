/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#pragma once

#include <QPointF>

#include <cstddef>

namespace mcc {
namespace ui {
namespace map {

struct LineIndexAndPos {
    LineIndexAndPos(std::size_t index, const QPointF& pos)
        : index(index)
        , pos(pos)
    {
    }

    std::size_t index;
    QPointF pos;
};
}
}
}
