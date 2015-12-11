/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include <QDebug>
#include "mcc/messages/MessageSender.h"
#include "mcc/encoder/core/ExchangeHelper.h"
#include "mcc/encoder/internal/Internal.h"
#include "mcc/encoder/internal/DeviceCommander.h"
#include "mcc/messages/Cmd.h"

namespace mcc {
namespace encoder {
namespace internal {

    DeviceCommander::DeviceCommander(const std::string& name, const std::string& device_name, const mcc::messages::MessageSender& out, std::size_t triesCount)
        : mcc::encoder::core::DeviceTask(name, device_name, out, triesCount)
    {
        _cmdsInQueue = 0;
        _cmdsProcessed = 0;
    }

    DeviceCommander::~DeviceCommander()
    {
        finish(true);
    }

    void DeviceCommander::push(std::unique_ptr<mcc::messages::Cmd>&& cmd)
    {
        std::lock_guard<std::mutex> lock(_mutex);
        _out->send<mcc::messages::CmdState>(*cmd, mcc::messages::CmdState::Value::WaitingInDeliveryQueue);
        _cmds.push_back(std::move(cmd));
        _cmdsInQueue = _cmds.size();
        _wakeup.notify_one();
    }

    void DeviceCommander::cancel(std::unique_ptr<mcc::messages::CmdCancel>&& cancel)
    {
        std::lock_guard<std::mutex> lock(_mutex);
        auto i = std::find_if(_cmds.begin(), _cmds.end(), [&cancel](const std::unique_ptr<mcc::messages::Cmd>& cmd){return cmd->cmdId() == cancel->cmdId(); });
        if (i == _cmds.end())
            return;

        _out->send<mcc::messages::CmdState>(*i->get(), mcc::messages::CmdState::Value::Failed, "canceled by user");
        _cmds.erase(i);
        _cmdsInQueue = _cmds.size();
    }

    std::size_t DeviceCommander::cmdsInQueue() const
    {
        return _cmdsInQueue;
    }

    std::size_t DeviceCommander::cmdsProcessed() const
    {
        return _cmdsProcessed;
    }

    void DeviceCommander::waitCommands_()
    {
        std::unique_lock<std::mutex> lock(_mutex);
        if (_cmds.empty())
            _wakeup.wait(lock, [&](){ return !isRunning_() || (_isActive && _cmdsInQueue>0); });
    }

    void DeviceCommander::tick()
    {
        waitCommands_();
        if (_cmdsInQueue == 0)
            return;

        std::unique_ptr<mcc::messages::Cmd> cmd;
        {
            std::unique_lock<std::mutex> lock(_mutex);
            if (_cmds.empty())
            {
                _cmdsInQueue = _cmds.size();
                return;
            }
            cmd = std::move(_cmds.front());
            _cmds.pop_front();
            _cmdsInQueue = _cmds.size();
        }

        auto r = execute_(*cmd);
        ++_cmdsProcessed;
    }

    bmcl::Option<std::string> DeviceCommander::execute_(const mcc::messages::Cmd& cmd)
    {
        auto r = InternalCoder::encodePacket(&cmd);
        if (r.isErr())
            return std::string(toString(r.takeErr()));

        mcc::encoder::core::ExchangeHelper<InternalCoder> exchanger(_innerChannel, _triesCount);
        auto out = exchanger.requestReply(r.take());
        if (out.isErr())
            return std::string(toString(out.takeErr()));
        return mcc::misc::None;
    }

}
}
}
