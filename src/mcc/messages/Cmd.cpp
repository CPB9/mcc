/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "mcc/misc/Helpers.h"
#include "mcc/messages/Cmd.h"
#include "mcc/messages/protobuf/Message.pb.h"

namespace mcc {
namespace messages {

MESSAGE_REQUEREMENT_DEFINITIONS(Cmd);
MESSAGE_REQUEREMENT_DEFINITIONS(CmdState);
MESSAGE_REQUEREMENT_DEFINITIONS(CmdCancel);
MESSAGE_REQUEREMENT_DEFINITIONS(CmdSubscribe_Request);
MESSAGE_REQUEREMENT_DEFINITIONS(CmdSubscribe_Response);

std::atomic<CmdId> Cmd::_uniqueId(0);

std::string Cmd::paramsAsString(const std::string& delimeter) const
{
    if (_params.empty())
        return std::string();

    std::string r = _params[0].stringify();
    for (std::size_t i = 1; i < _params.size(); ++i)
    {
        r += delimeter + _params[i].stringify();
    }
    return r;
}

void Cmd::serialize_(mcc::protobuf::MessageBody* body) const
{
    auto cmd = body->mutable__cmd();
    cmd->set_device(_device);
    cmd->set_trait(_trait);
    cmd->set_command(_command);
    cmd->set_collationid(_cmdId);
    for (const auto& i : _params)
        *cmd->mutable_params()->Add() = i.serialize();
}

std::unique_ptr<Message> Cmd::deserialize(const mcc::protobuf::Cmd& cmd)
{
    mcc::misc::CmdParams params;
    params.reserve(cmd.params().size());
    for (const auto& i : cmd.params())
    {
        bmcl::MemReader reader(i.data(), i.size());
        params.emplace_back(mcc::misc::NetVariant::deserialize(&reader).take());
    }
    return mcc::misc::makeUnique<Cmd>(cmd.collationid(), cmd.device(), cmd.trait(), cmd.command(), std::move(params));
}


}
}

