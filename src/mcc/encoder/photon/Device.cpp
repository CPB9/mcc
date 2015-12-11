/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include <QDebug>
#include "bmcl/Result.h"

#include "mcc/messages/MessageSender.h"
#include "mcc/encoder/photon/Device.h"
#include "mcc/encoder/photon/DeviceController.h"


namespace mcc {
namespace encoder {
namespace photon {

    Device::Device(const mcc::misc::ProtocolId& id, const mcc::messages::MessageSender& out, const std::string& device_name)
        : mcc::encoder::core::Device(id, device_name, out)
    {
        registrate_();
        _controller = mcc::misc::Runnable::startInThread<DeviceController>(device_name + ".cmd", device_name, _out, 3);
        addTask_(_controller);
    }

    Device::~Device()
    {
    }

    void Device::activate(bool state)
    {
        activate_(state);
        if (!isActive())
            return;
    }

    float Device::regState() const
    {
        if (isRegistered())
            return 100.0;
        return 0.0;
    }

    void Device::pushCmd(std::unique_ptr<mcc::messages::Cmd>&& cmd)
    {
        _controller->pushCmd(std::move(cmd));
    }

    void Device::cancelCmd(std::unique_ptr<mcc::messages::CmdCancel>&& cancel)
    {
        if (_controller)
            _controller->cancelCmd(std::move(cancel));
    }

    std::size_t Device::cmdsInQueue() const
    {
        if (_controller)
            return _controller->cmdsInQueue();
        return 0;
    }

    std::size_t Device::cmdsProcessed() const
    {
        if (_controller)
            return _controller->cmdsProcessed();
        return 0;
    }

    void Device::asyncAccept(const std::vector<uint8_t>& packet)
    {
        if (!isActive())
            return;
        _controller->_outerChannel.sender.send(packet);
    }
}
}
}
