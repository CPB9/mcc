/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#pragma once
#include <memory>
#include "bmcl/MemReader.h"
#include "bmcl/MemWriter.h"
#include "bmcl/Result.h"

#include "mcc/messages/MessageSender.h"
#include "mcc/misc/Device.h"
#include "mcc/misc/Channel.h"
#include "mcc/misc/Cmd.h"
#include "mcc/misc/NetStatistics.h"
#include "mcc/encoder/core/Device.h"


namespace mcc {
namespace encoder {
namespace photon {

class DeviceController;

class Device : public mcc::encoder::core::Device
{
public:
    Device(const mcc::misc::ProtocolId& id, const mcc::messages::MessageSender& out, const std::string& device_name);
    virtual ~Device();

    void asyncAccept(const std::vector<uint8_t>& packet) override;
    void activate(bool isActive) override;
    float regState() const override;
    void pushCmd(std::unique_ptr<mcc::messages::Cmd>&& cmd) override;
    void cancelCmd(std::unique_ptr<mcc::messages::CmdCancel>&& cancel) override;
    std::size_t cmdsInQueue() const override;
    std::size_t cmdsProcessed() const override;

private:

    std::shared_ptr<DeviceController> _controller;
};

}
}
}
