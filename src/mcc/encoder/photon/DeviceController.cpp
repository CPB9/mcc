/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "mcc/messages/MessageSender.h"
#include "mcc/messages/Cmd.h"
#include "mcc/encoder/core/ExchangeHelper.h"
#include "mcc/encoder/photon/Photon.h"
#include "mcc/encoder/photon/DeviceController.h"


namespace mcc {
namespace encoder {
namespace photon {

    DeviceController::DeviceController(const std::string& name, const std::string& device_name, const mcc::messages::MessageSender& out, std::size_t triesCount)
        : mcc::encoder::core::DeviceTask(name, device_name, out, triesCount)
    {
        _cmdsProcessed = 0;
    }

    DeviceController::~DeviceController()
    {
        finish(true);
    }

    void DeviceController::pushCmd(std::unique_ptr<mcc::messages::Cmd>&& cmd)
    {
        ++_cmdsProcessed;
    }

    void DeviceController::cancelCmd(std::unique_ptr<mcc::messages::CmdCancel>&& cancel)
    {
        Q_UNUSED(cancel);
    }

    void DeviceController::tick()
    {
        auto r = _innerChannel.reciever.tryRecvFor(std::chrono::milliseconds(100));
        if (r.isOk())
        {
            auto tm = r.take();
            auto t = PhotonCoder::decode(tm.data(), tm.size());
            if (t.isErr())
            {
                toString(t.takeErr());
            }
            else
            {
            }
        }
    }

    bmcl::Option<std::string> DeviceController::execute_(const mcc::messages::Cmd& cmd)
    {
        Q_UNUSED(cmd);
        return bmcl::None;
    }



}
}
}
