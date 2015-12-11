/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#pragma once

namespace mcc {
namespace ui {
namespace map {

struct TilePosition {
    TilePosition(int z, int x, int y)
    {
        zoomLevel = z;
        globalOffsetX = x;
        globalOffsetY = y;
    }

    TilePosition()
        : TilePosition(1, 0, 0)
    {
    }

    int zoomLevel;
    int globalOffsetX;
    int globalOffsetY;
};

inline bool operator==(const TilePosition& left, const TilePosition& right)
{
    return left.zoomLevel == right.zoomLevel && left.globalOffsetX == right.globalOffsetX
        && left.globalOffsetY == right.globalOffsetY;
}
}
}
}
