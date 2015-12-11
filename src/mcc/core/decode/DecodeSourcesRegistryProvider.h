/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#pragma once

#include <iostream>
#include <memory>

#include "bmcl/Assert.h"
#include "bmcl/Logging.h"

#include "mcc/core/decode/Registry.h"
#include "mcc/core/decode/RegistryProvider.h"

namespace mcc
{
namespace decode
{
    
class DecodeSourcesRegistryProvider : public RegistryProvider
{
public:
    explicit DecodeSourcesRegistryProvider(std::initializer_list<std::string> sources) : _sources(sources) {}
    std::unique_ptr<Registry> provide() override;
private:
    std::vector<std::string> _sources;
};
    
}
}