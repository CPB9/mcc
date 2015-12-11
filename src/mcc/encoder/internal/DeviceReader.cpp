/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include <string>
#include <QDebug>
#include "bmcl/MemWriter.h"
#include "bmcl/Result.h"

#include "mcc/messages/MessageSender.h"
#include "mcc/messages/Cmd.h"
#include "mcc/encoder/core/ExchangeHelper.h"
#include "mcc/encoder/internal/Internal.h"
#include "mcc/encoder/internal/DeviceReader.h"


namespace mcc {
namespace encoder {
namespace internal {

    DeviceReader::DeviceReader(const std::string& name, const std::string& device_name, const mcc::messages::MessageSender& out, std::size_t triesCount)
        : DeviceTask(name, device_name, out, triesCount)
    {
    }

    DeviceReader::~DeviceReader()
    {
        finish(true);
    }

    void DeviceReader::push(std::unique_ptr<mcc::messages::Cmd>&& cmd)
    {
        const auto& params = cmd->params();
        if (cmd->command() == "startReading")
        {
            addTrait_(params[0].stringify());
            _out->send<mcc::messages::CmdState>(*cmd, mcc::messages::CmdState::Value::AcknowledgeReceived);
        }
        else if (cmd->command() == "stopReading")
        {
            removeTrait_(params[0].stringify());
            _out->send<mcc::messages::CmdState>(*cmd, mcc::messages::CmdState::Value::AcknowledgeReceived);
        }
        else
        {
            _out->send<mcc::messages::CmdState>(*cmd, mcc::messages::CmdState::Value::Failed, "command unknown");
        }
    }

    void DeviceReader::addTrait_(const std::string& trait)
    {
        if (trait.empty())
            return;

        std::unique_lock<std::mutex> lock(_mutex);
        auto i = std::find(_traits.begin(), _traits.end(), trait);
        if (i == _traits.end())
        {
            _traits.emplace_back(trait);
        }

        _wakeup.notify_one();
    }

    void DeviceReader::removeTrait_(const std::string& trait)
    {
        std::unique_lock<std::mutex> lock(_mutex);
        _traitsToRemove.emplace(trait);
    }

    std::size_t DeviceReader::waitTraits_()
    {
        std::unique_lock<std::mutex> lock(_mutex);
        if (_traits.empty())
            _wakeup.wait(lock);
        return _traits.size();
    }

    void DeviceReader::removeTraits_()
    {
        std::unique_lock<std::mutex> lock(_mutex);
        if (_traitsToRemove.empty())
            return;

        for (const auto& i: _traitsToRemove)
        {
            _traits.erase(std::remove_if(_traits.begin(), _traits.end(), [=](const std::string& trait){ return trait == i; }), _traits.end());
        }
        _traitsToRemove.clear();
    }

    void DeviceReader::tick()
    {
        mcc::encoder::core::ExchangeHelper<InternalCoder> exchanger(_innerChannel, _triesCount);
        mcc::messages::Cmd cmd("", "Tm", "readOnce");
        auto r = exchanger.requestReply<InternalCoder>(cmd);
        if (r.isOk())
        {
            _out->send(r.take());
        }
        else
        {
            auto err = r.takeErr();
        }

        removeTraits_();
    }

}
}
}
