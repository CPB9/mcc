/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include <QDebug>
#include "mcc/messages/Message.h"
#include "mcc/messages/Cmd.h"
#include "mcc/messages/Tm.h"
#include "mcc/messages/protobuf/Message.pb.h"


namespace mcc {
namespace messages {

MessagePtr Message::deserialize(const void* ptr, std::size_t size)
{
    mcc::protobuf::Message msg;
    if (!msg.ParseFromArray(ptr, size))
    {
        return nullptr;
    }

    MessagePtr p;
    switch (msg.body().body_case())
    {
    case mcc::protobuf::MessageBody::kCmd: p = Cmd::deserialize(msg.body()._cmd()); break;
    case mcc::protobuf::MessageBody::kTmParamList: p = TmParamList::deserialize(msg.body()._tmparamlist()); break;
    default:
        assert(false);
        return nullptr;
    }

    p->_id = msg.header().id();
    p->_receiver = msg.header().receiver();
    p->_sender = msg.header().sender();
    p->_time = msg.header().time();
    if (msg.header().has_requestid())
        p->_requestId = msg.header().requestid();

    return p;
}

std::vector<uint8_t> Message::serialize() const
{
    mcc::protobuf::Message msg;
    serialize_(msg.mutable_body());

    auto header = msg.mutable_header();
    header->set_id(message_id());
    header->set_receiver(receiver());
    header->set_sender(sender());
    header->set_body(msg.body().body_case());
    header->set_time(time());
    if (requestId().isSome())
        header->set_requestid(*requestId());

    std::vector<uint8_t> buffer;
    buffer.resize(msg.ByteSize());
    msg.SerializeToArray(buffer.data(), buffer.size());
    return buffer;
}

void Message::serialize_(mcc::protobuf::MessageBody* body) const
{
    /*not implemented*/
    assert(false);
}


MESSAGE_REQUEREMENT_DEFINITIONS(Error);

MessageProcessor::MessageProcessor(const MessageSender& sender) : _sender(sender)
{
}

void MessageProcessor::chooseProcessor(MessagePtr&& message, MessageProcessor* processor)
{
    message->process(processor, std::move(message));
}

void MessageProcessor::chooseProcessor(MessagePtr&& message)
{
    message->process(this, std::move(message));
}

void MessageProcessor::not_implemented_(std::unique_ptr<Message>&& msg)
{
    qDebug() << QString::fromStdString(msg->message_name()) << QString::fromStdString(msg->sender()) << "->" << QString::fromStdString(msg->receiver());
}

void MessageProcessor::process(std::unique_ptr<mcc::messages::Error>&& msg){ /*stop recursion*/ /*not_implemented_();*/ }
void MessageProcessor::process(std::unique_ptr<SystemState>&& msg){ not_implemented_(Message::to_base(msg)); }
void MessageProcessor::process(std::unique_ptr<SystemState_Request>&& msg){ not_implemented_(Message::to_base(msg)); }
void MessageProcessor::process(std::unique_ptr<SystemComponentState>&& msg){ not_implemented_(Message::to_base(msg)); }
void MessageProcessor::process(std::unique_ptr<SystemComponentState_Request>&& msg){ not_implemented_(Message::to_base(msg)); }

void MessageProcessor::process(std::unique_ptr<Cmd>&& msg){ not_implemented_(Message::to_base(msg)); }
void MessageProcessor::process(std::unique_ptr<CmdCancel>&& msg){ not_implemented_(Message::to_base(msg)); }
void MessageProcessor::process(std::unique_ptr<CmdState>&& msg){ not_implemented_(Message::to_base(msg)); }
void MessageProcessor::process(std::unique_ptr<CmdSubscribe_Request>&& msg){ not_implemented_(Message::to_base(msg)); }
void MessageProcessor::process(std::unique_ptr<CmdSubscribe_Response>&& msg){ not_implemented_(Message::to_base(msg)); }

void MessageProcessor::process(std::unique_ptr<DeviceActivate_Request>&& msg){ not_implemented_(Message::to_base(msg)); }
void MessageProcessor::process(std::unique_ptr<DeviceActivate_Response>&& msg){ not_implemented_(Message::to_base(msg)); }
void MessageProcessor::process(std::unique_ptr<DeviceConnect_Request>&& msg){ not_implemented_(Message::to_base(msg)); }
void MessageProcessor::process(std::unique_ptr<DeviceConnect_Response>&& msg){ not_implemented_(Message::to_base(msg)); }
void MessageProcessor::process(std::unique_ptr<DeviceDisconnect_Request>&& msg){ not_implemented_(Message::to_base(msg)); }
void MessageProcessor::process(std::unique_ptr<DeviceDisconnect_Response>&& msg){ not_implemented_(Message::to_base(msg)); }
void MessageProcessor::process(std::unique_ptr<DeviceFileLoad_Request>&& msg){ not_implemented_(Message::to_base(msg)); }
void MessageProcessor::process(std::unique_ptr<DeviceFileLoad_Response>&& msg){ not_implemented_(Message::to_base(msg)); }
void MessageProcessor::process(std::unique_ptr<DeviceFileLoadCancel_Request>&& msg){ not_implemented_(Message::to_base(msg)); }
void MessageProcessor::process(std::unique_ptr<DeviceList_Request>&& msg){ not_implemented_(Message::to_base(msg)); }
void MessageProcessor::process(std::unique_ptr<DeviceList_Response>&& msg){ not_implemented_(Message::to_base(msg)); }
void MessageProcessor::process(std::unique_ptr<DeviceDescription_Request>&& msg){ not_implemented_(Message::to_base(msg)); }
void MessageProcessor::process(std::unique_ptr<DeviceDescription_Response>&& msg){ not_implemented_(Message::to_base(msg)); }
void MessageProcessor::process(std::unique_ptr<DeviceRegister_Request>&& msg){ not_implemented_(Message::to_base(msg)); }
void MessageProcessor::process(std::unique_ptr<DeviceRegister_Response>&& msg){ not_implemented_(Message::to_base(msg)); }
void MessageProcessor::process(std::unique_ptr<DeviceRegistered>&& msg){ not_implemented_(Message::to_base(msg)); }
void MessageProcessor::process(std::unique_ptr<DeviceUnRegistered>&& msg){ not_implemented_(Message::to_base(msg)); }
void MessageProcessor::process(std::unique_ptr<DeviceUpdate_Request>&& msg){ not_implemented_(Message::to_base(msg)); }
void MessageProcessor::process(std::unique_ptr<DeviceUpdate_Response>&& msg){ not_implemented_(Message::to_base(msg)); }
void MessageProcessor::process(std::unique_ptr<DeviceUpdated>&& msg){ not_implemented_(Message::to_base(msg)); }
void MessageProcessor::process(std::unique_ptr<DeviceUnRegister_Request>&& msg){ not_implemented_(Message::to_base(msg)); }
void MessageProcessor::process(std::unique_ptr<DeviceUnRegister_Response>&& msg){ not_implemented_(Message::to_base(msg)); }
void MessageProcessor::process(std::unique_ptr<DeviceState_Response>&& msg){ not_implemented_(Message::to_base(msg)); }
void MessageProcessor::process(std::unique_ptr<DeviceActionLog>&& msg){ not_implemented_(Message::to_base(msg)); }

void MessageProcessor::process(std::unique_ptr<FirmwareList_Request>&& msg){ not_implemented_(Message::to_base(msg)); }
void MessageProcessor::process(std::unique_ptr<FirmwareList_Response>&& msg){ not_implemented_(Message::to_base(msg)); }
void MessageProcessor::process(std::unique_ptr<FirmwareDescription_Request>&& msg){ not_implemented_(Message::to_base(msg)); }
void MessageProcessor::process(std::unique_ptr<FirmwareDescription_Response>&& msg){ not_implemented_(Message::to_base(msg)); }
void MessageProcessor::process(std::unique_ptr<FirmwareRegister_Request>&& msg){ not_implemented_(Message::to_base(msg)); }
void MessageProcessor::process(std::unique_ptr<FirmwareRegister_Response>&& msg){ not_implemented_(Message::to_base(msg)); }
void MessageProcessor::process(std::unique_ptr<FirmwareRegistered>&& msg){ not_implemented_(Message::to_base(msg)); }

void MessageProcessor::process(std::unique_ptr<Channel_Request>&& msg){ not_implemented_(Message::to_base(msg)); }
void MessageProcessor::process(std::unique_ptr<Channel_Response>&& msg){ not_implemented_(Message::to_base(msg)); }
void MessageProcessor::process(std::unique_ptr<ChannelState_Response>&& msg){ not_implemented_(Message::to_base(msg)); }

void MessageProcessor::process(std::unique_ptr<TmParamSubscribe_Request>&& msg){ not_implemented_(Message::to_base(msg)); }
void MessageProcessor::process(std::unique_ptr<TmParamSubscribe_Response>&& msg){ not_implemented_(Message::to_base(msg)); }
void MessageProcessor::process(std::unique_ptr<TmParamList>&& msg){ not_implemented_(Message::to_base(msg)); }

void MessageProcessor::process(std::unique_ptr<ProtocolList_Request>&& msg){ not_implemented_(Message::to_base(msg)); }
void MessageProcessor::process(std::unique_ptr<ProtocolList_Response>&& msg){ not_implemented_(Message::to_base(msg)); }
void MessageProcessor::process(std::unique_ptr<ProtocolDescription_Request>&& msg){ not_implemented_(Message::to_base(msg)); }
void MessageProcessor::process(std::unique_ptr<ProtocolDescription_Response>&& msg){ not_implemented_(Message::to_base(msg)); }
void MessageProcessor::process(std::unique_ptr<ProtocolForDevice_Request>&& msg){ not_implemented_(Message::to_base(msg)); }
void MessageProcessor::process(std::unique_ptr<ProtocolForDevice_Response>&& msg){ not_implemented_(Message::to_base(msg)); }
void MessageProcessor::process(std::unique_ptr<ProtocolForDeviceRegister_Request>&& msg){ not_implemented_(Message::to_base(msg)); }
void MessageProcessor::process(std::unique_ptr<ProtocolForDeviceRegister_Response>&& msg){ not_implemented_(Message::to_base(msg)); }

}
}

