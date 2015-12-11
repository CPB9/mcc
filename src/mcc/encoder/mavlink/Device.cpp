/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include <QDebug>
#include "bmcl/Result.h"

#include "mcc/messages/MessageSender.h"
#include "mcc/encoder/mavlink/Device.h"
#include "mcc/encoder/mavlink/DeviceController.h"


namespace mcc {
namespace encoder {
namespace mavlink {

    Device::Device(const mcc::misc::ProtocolId& id, const mcc::messages::MessageSender& out, const std::string& device_name)
        : mcc::encoder::core::Device(id, device_name, out)
    {
        registrate_();
        _commander = mcc::misc::Runnable::startInThread<DeviceController>(device_name + ".cmd", device_name, _out, 3);
        addTask_(_commander);
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
        _commander->push(std::move(cmd));
    }

    void Device::cancelCmd(std::unique_ptr<mcc::messages::CmdCancel>&& cancel)
    {
        if (_commander)
            _commander->cancel(std::move(cancel));
    }

    std::size_t Device::cmdsInQueue() const
    {
        if (_commander)
            return _commander->cmdsInQueue();
        return 0;
    }

    std::size_t Device::cmdsProcessed() const
    {
        if (_commander)
            return _commander->cmdsProcessed();
        return 0;
    }

    void Device::asyncAccept(const std::vector<uint8_t>& packet)
    {
        if (!isActive())
            return;
        std::lock_guard<std::mutex> lock(_mutex);
        if (!packet.empty())
            _stats._rcvd.add(packet.size());

        _commander->_outerChannel.sender.send(packet);
    }

}
}
}
