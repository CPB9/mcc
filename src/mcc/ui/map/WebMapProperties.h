/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#pragma once

#include <cstddef>

namespace mcc {
namespace ui {
namespace map {

class WebMapProperties {
public:
    static std::size_t tilePixelSize()
    {
        return 256;
    }

    static std::size_t minZoom()
    {
        return 1;
    }

    static std::size_t maxZoom()
    {
        return 23;
    }
};
}
}
}
