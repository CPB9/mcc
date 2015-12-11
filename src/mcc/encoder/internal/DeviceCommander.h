/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#pragma once
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
namespace internal {

class DeviceCommander : public mcc::encoder::core::DeviceTask
{
public:
    DeviceCommander(const std::string& name, const std::string& device_name, const mcc::messages::MessageSender& out, std::size_t triesCount);
    virtual ~DeviceCommander();
    void tick() override;
    void push(std::unique_ptr<mcc::messages::Cmd>&& cmd);
    void cancel(std::unique_ptr<mcc::messages::CmdCancel>&& cancel);
    std::size_t cmdsInQueue() const;
    std::size_t cmdsProcessed() const;

private:
    void waitCommands_();

    bmcl::Option<std::string> execute_(const mcc::messages::Cmd& cmd);

    std::atomic<std::size_t> _cmdsInQueue;
    std::atomic<std::size_t> _cmdsProcessed;

    std::deque<std::unique_ptr<mcc::messages::Cmd>> _cmds;
};

typedef std::shared_ptr<DeviceCommander> DeviceCommanderPtr;

}
}
}
