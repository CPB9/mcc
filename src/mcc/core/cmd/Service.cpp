/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include <QCoreApplication>
#include <QDebug>

#include "bmcl/Logging.h"
#include "mcc/Names.h"
#include "mcc/core/cmd/Service.h"
#include "mcc/messages/Cmd.h"
#include "mcc/messages/MessageSender.h"


namespace mcc {
namespace core {
namespace cmd {

Service::Command::Command(const std::string& from, const std::string& registered, const mcc::messages::Cmd& cmd)
    :_from(from), _registered(registered), _state(mcc::misc::CmdState::Registered)
{
}

Service::Service(const mcc::messages::LocalRouterPtr& router) : mcc::messages::ServiceAbstract(mcc::Names::coreCmd(), router)
{
}

Service::~Service()
{
    finish(true);
}

void Service::process(std::unique_ptr<mcc::messages::Cmd>&& cmd)
{
    auto dev = _devices.find(cmd->device());
    if (dev == _devices.end())
    {
        _out->sendTo<mcc::messages::CmdState>(cmd->sender(), *cmd, mcc::messages::CmdState::Failed, "unknown device");
        return;
    }

    Q_ASSERT(dev->second._cmds.find(cmd->cmdId()) == dev->second._cmds.end());
    dev->second._cmds.emplace(cmd->cmdId(), Command(cmd->sender(), cmd->time(), *cmd));
    if (dev->second._cmds.size() > _CmdsLimit)
    {
        _out->sendTo<mcc::messages::CmdState>(cmd->sender(), *cmd, mcc::messages::CmdState::Value::Failed, "too many commands waiting");
        return;
    }

    if (dev->second._exchanger.empty())
    {
        _out->sendTo<mcc::messages::CmdState>(cmd->sender(), *cmd, mcc::messages::CmdState::Value::Failed, "exchanger for device is not set");
        return;
    }

    _out->sendTo(mcc::Names::coreDb(), cmd->clone());
    _out->sendTo(dev->second._exchanger, std::move(cmd));
}

void Service::process(std::unique_ptr<mcc::messages::CmdCancel>&& cancel)
{
    auto dev = _devices.find(cancel->device());
    if (dev == _devices.end())
    {
        Q_ASSERT(false);
        return;
    }

    const auto& i = dev->second._cmds.find(cancel->cmdId());
    if (i == dev->second._cmds.end())
        return;

    const auto& cmd = i->second;
    if (cmd._state == mcc::misc::CmdState::AcknowledgeReceived || cmd._state == mcc::misc::CmdState::Failed)
        return;

    _out->sendTo(dev->second._exchanger, std::move(cancel));
}

void Service::process(std::unique_ptr<mcc::messages::CmdState>&& state)
{
    const auto& device = state->device();
    auto i = _devices[device]._cmds.find(state->cmdId());
    if (i == _devices[device]._cmds.end())
    {
        BMCL_DEBUG() << "Acknowledge for unknown command " << state->cmdId() << mcc::misc::toString((mcc::misc::CmdState)state->state()) << "(maybe late)";
        return;
    }

    auto& cmd = i->second;
    cmd._state = (mcc::misc::CmdState)state->state();
    _out->sendTo(mcc::Names::coreDb(), state->clone());
    _out->sendTo(cmd._from, std::move(state));

    if (cmd._state == mcc::misc::CmdState::AcknowledgeReceived || cmd._state == mcc::misc::CmdState::Failed)
        _devices[device]._cmds.erase(i);

}

void Service::process(std::unique_ptr<mcc::messages::CmdSubscribe_Request>&& subscription)
{
    if (subscription->isOn())
    {
        _devices[subscription->device()];
        if (_devices.find(subscription->device()) != _devices.end())
        {
            _devices[subscription->device()]._exchanger = subscription->sender();
            _out->respond<mcc::messages::CmdSubscribe_Response>(subscription.get());
        }
        else
        {
            _out->respond<mcc::messages::CmdSubscribe_Response>(subscription.get(), "device is not registered");
        }
    }
    else
    {
        _devices[subscription->device()]._exchanger.clear();
    }
}

}
}
}
