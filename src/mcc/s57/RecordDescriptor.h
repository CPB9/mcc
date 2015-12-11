/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#pragma once

#include "mcc/s57/Fields.h"
#include "mcc/s57/Directory.h"

#include <cstdint>
#include <vector>
#include <array>

namespace mcc {
namespace s57 {

struct Leader {
    uint64_t length;
    char stamp[8];
    uint64_t fieldOffset;
    uint8_t fieldLengthSize;
    uint8_t fieldPositionSize;
    uint8_t fieldTagSize;

    unsigned fieldDescriptionSize() const
    {
        return fieldPositionSize + fieldLengthSize + fieldTagSize;
    }
};
}
}
