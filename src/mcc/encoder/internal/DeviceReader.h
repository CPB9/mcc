/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#pragma once
#include <set>
#include <vector>

#include "mcc/messages/Deaclarations.h"
#include "mcc/encoder/core/DeviceTask.h"


namespace mcc {
namespace encoder {
namespace internal {

class DeviceReader : public mcc::encoder::core::DeviceTask
{
public:
    DeviceReader(const std::string& name, const std::string& device_name, const mcc::messages::MessageSender& out, std::size_t triesCount);
    virtual ~DeviceReader();

    void push(std::unique_ptr<mcc::messages::Cmd>&& cmd);
    void tick() override;

private:
    void addTrait_(const std::string& trait);
    void removeTrait_(const std::string& trait);
    void removeTraits_();
    std::size_t waitTraits_();

    std::set<std::string> _traitsToRemove;
    std::vector<std::string> _traits;
};

typedef std::shared_ptr<DeviceReader> DeviceReaderPtr;

}
}
}
