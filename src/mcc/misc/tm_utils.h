/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#pragma once

#include <string>

#include "mcc/messages/Cmd.h"
#include "mcc/messages/MessageSender.h"

namespace mcc
{
namespace misc
{
void sendCommandStateEvent(mcc::messages::MessageSender & messageSender, const mcc::messages::Cmd& cmd,
    mcc::messages::CmdState::Value state, const std::string & reason = std::string());
}
}
