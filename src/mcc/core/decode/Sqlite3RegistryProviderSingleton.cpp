/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "mcc/core/decode/Sqlite3RegistryProviderSingleton.h"

namespace mcc
{
namespace decode
{
std::shared_ptr<Component> findDecodeComponentOrFail(std::string componentFqn)
{
    std::shared_ptr<Component> s = Sqlite3RegistryProviderSingleton::getInstance()->componentByFqn(componentFqn);
    BMCL_ASSERT(s);
    return s;
}
}
}
