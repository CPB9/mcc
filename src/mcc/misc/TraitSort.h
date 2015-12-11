/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#pragma once

#include <cstdint>
#include <vector>
#include <map>
#include "mcc/misc/Firmware.h"

namespace mcc {
    namespace misc {
        struct NodeLevel
        {
            NodeLevel(){}
            NodeLevel(uint16_t n, uint8_t l, uint16_t i) :node(n), level(l), index(i){}
            uint16_t node;
            uint8_t  level;
            uint16_t index;
        };

        std::vector<NodeLevel> sortTraits(const mcc::misc::TraitDescriptionList& traits);
    }
}
