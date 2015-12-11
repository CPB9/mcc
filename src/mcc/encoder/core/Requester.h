/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#pragma once
#include <vector>
#include "mcc/misc/Channel.h"


namespace mcc {
namespace encoder {
namespace core {

typedef mcc::misc::ChannelPair<std::vector<uint8_t>> Requester;

}}}