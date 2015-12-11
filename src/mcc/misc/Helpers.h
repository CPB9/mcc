/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#pragma once

#include <memory>

namespace mcc {
namespace misc {

template <typename T, typename... A>
inline std::unique_ptr<T> makeUnique(A&&... args)
{
    return std::unique_ptr<T>(new T(std::forward<A>(args)...));
}
}
}
