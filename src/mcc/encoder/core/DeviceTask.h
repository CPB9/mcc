/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#pragma once
#include <mutex>
#include <memory>
#include <future>
#include <string>
#include "bmcl/Result.h"

#include "mcc/misc/Channel.h"
#include "mcc/misc/Device.h"
#include "mcc/misc/Firmware.h"
#include "mcc/misc/Net.h"
#include "mcc/messages/Deaclarations.h"
#include "mcc/misc/Runnable.h"
#include "mcc/encoder/core/Requester.h"


namespace mcc {
namespace encoder {
namespace core {


class DeviceTask : public mcc::misc::Runnable
{
public:
    DeviceTask(const std::string& name, const std::string& device_name, const mcc::messages::MessageSender& out, std::size_t triesCount = std::size_t(-1))
        : Runnable(name)
        , _triesCount(triesCount)
        , _out(out)
        , _device_name(device_name)
    {
        _isActive = false;
        auto bi = mcc::misc::makeBiChannel<std::vector<uint8_t>>();
        _innerChannel = bi.forward;
        _outerChannel = bi.backward;
    }
    virtual ~DeviceTask()
    {
        _outerChannel.sender.close();
        _innerChannel.sender.close();
        finish(true);
    }

    virtual bool pre() override { return true; }
    virtual void post() override { }

    virtual void activate(bool state) { _isActive = state; _wakeup.notify_all(); }
    virtual void updateFirmware(const mcc::misc::FirmwareDescriptionPtr& firmware) { Q_UNUSED(firmware); };

    Requester   _outerChannel;

protected:

    std::size_t _triesCount = 0;
    Requester   _innerChannel;
    std::string _device_name;
    mcc::messages::MessageSender   _out;
    std::atomic<bool>           _isActive;
};

}
}
}
