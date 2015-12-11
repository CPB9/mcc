/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "mcc/misc/tm_utils.h"

#include <QDebug>

using mcc::messages::MessageSender;
using mcc::messages::CmdState;

namespace mcc
{
namespace misc
{
void sendCommandStateEvent(mcc::messages::MessageSender & messageSender, const mcc::messages::Cmd& cmd,
    mcc::messages::CmdState::Value state, const std::string & reason)
{
    if (state == messages::CmdState::Value::Failed)
        qDebug() << cmd.name().c_str() << " не выполнена: " << reason.c_str();
    else if (state == messages::CmdState::Value::AcknowledgeReceived)
        qDebug() << cmd.name().c_str() << " выполнена";
    messageSender->send<mcc::messages::CmdState>(cmd, state, reason);
}
}
}
