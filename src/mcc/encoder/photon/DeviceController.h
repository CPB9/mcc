/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#pragma once
#include <memory>
#include <string>
#include "bmcl/MemWriter.h"
#include "bmcl/Result.h"
#include "bmcl/Option.h"

#include "mcc/misc/Cmd.h"
#include "mcc/misc/Device.h"
#include "mcc/messages/Deaclarations.h"
#include "mcc/encoder/core/DeviceTask.h"


namespace mcc {
namespace encoder {
namespace photon {

class DeviceController : public mcc::encoder::core::DeviceTask
{
public:
    DeviceController(const std::string& name, const std::string& device_name, const mcc::messages::MessageSender& out, std::size_t triesCount);
    virtual ~DeviceController();
    void tick() override;

    void pushCmd(std::unique_ptr<mcc::messages::Cmd>&& cmd);
    void cancelCmd(std::unique_ptr<mcc::messages::CmdCancel>&& cancel);
    std::size_t cmdsInQueue() const { return 0; }
    std::size_t cmdsProcessed() const { return _cmdsProcessed; }

private:

    bmcl::Option<std::string> execute_(const mcc::messages::Cmd& cmd);
    std::size_t _cmdsProcessed;
};

typedef std::shared_ptr<DeviceController> DeviceControllerPtr;

}
}
}
