/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#pragma once

#include "bmcl/Assert.h"

namespace mcc
{
namespace utils
{

template<typename T, typename F>
inline T checked_cast(F value)
{
    BMCL_ASSERT(std::numeric_limits<T>::min() <= value && std::numeric_limits<T>::max() >= value);
    return static_cast<T>(value);
};

}
}
