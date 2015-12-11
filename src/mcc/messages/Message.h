/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#pragma once
#include <string>
#include <memory>
#include <vector>
#include <atomic>
#include "bmcl/Option.h"
#include "mcc/Names.h"
#include "mcc/misc/Helpers.h"
#include "mcc/messages/Deaclarations.h"

namespace mcc { namespace core { namespace router { class Service; } } }

namespace mcc { namespace protobuf { class MessageBody; } }

namespace mcc {
namespace messages {


class Message
{
    friend class MessageSenderX;
    friend class mcc::core::router::Service;
public:
    Message() = default;
    Message(const Message&) = default;
    Message(const std::string& to) : _receiver(to){}
    Message(const std::string& to, MessageId requestId) : _receiver(to), _requestId(requestId){}
    virtual ~Message(){}
    const std::string& sender() const { return _sender; }
    const std::string& receiver() const { return _receiver; }
    const std::string& time() const { return _time; }
    MessageId message_id() const { return _id; }
    bmcl::Option<MessageId> requestId() const { return _requestId; }
    static std::unique_ptr<Message> deserialize(const void* ptr, std::size_t size);
    template<class T>
    static std::unique_ptr<Message> to_base(T&& msg) { return std::unique_ptr<Message>((Message*)msg.release()); }

    virtual const std::string& message_name() const = 0;
    virtual void process(MessageProcessor* processor, std::unique_ptr<Message>&& message) = 0;
    virtual std::unique_ptr<Message> clone() const = 0;
    std::vector<uint8_t> serialize() const;

protected:
    virtual void serialize_(mcc::protobuf::MessageBody* body) const;

private:

    MessageId   _id;
    std::string _sender;
    std::string _receiver;
    std::string _time;
    bmcl::Option<MessageId> _requestId;
};
typedef std::unique_ptr<Message> MessagePtr;

class MessageTo : public Message
{
public:
    MessageTo(const std::string& to) : Message(to){}
    virtual ~MessageTo(){}
};

class Response : public Message
{
public:
    Response(const Message* request) : Message(request->sender(), request->message_id()){}
    virtual ~Response(){}
};


#define MESSAGE_REQUIREMENTS_DECLARATIONS() \
    const std::string& message_name() const override { return _message_name; } \
    void process(MessageProcessor* processor, std::unique_ptr<Message>&& message) override; \
    std::unique_ptr<Message> clone() const override;    \
    static const std::string _message_name;

#define MESSAGE_REQUEREMENT_DEFINITIONS(TypeName) \
    void TypeName::process(MessageProcessor* processor, std::unique_ptr<Message>&& message){ processor->process(std::unique_ptr<TypeName>(static_cast<TypeName*>(message.release()))); } \
    std::unique_ptr<Message> TypeName::clone() const { return std::unique_ptr<Message>(new TypeName(*this)); } \
    const std::string TypeName::_message_name = #TypeName;


class Error : public Message
{
public:
    Error(const Message* msg, const std::string& error) :_error(error){ (void)msg;  (void)_error; }
    virtual ~Error(){}
    MESSAGE_REQUIREMENTS_DECLARATIONS();
    const std::string& error() const { return _error; }
private:
    std::string _error;
};

class MessageProcessor
{
private:
    void not_implemented_(std::unique_ptr<Message>&&);
    MessageSender _sender;
public :
    MessageProcessor(const MessageSender& sender);
    static void chooseProcessor(MessagePtr&&, MessageProcessor* processor);
    void chooseProcessor(MessagePtr&&);

    virtual void process(std::unique_ptr<Error>&&);
    virtual void process(std::unique_ptr<SystemState>&&);
    virtual void process(std::unique_ptr<SystemState_Request>&&);
    virtual void process(std::unique_ptr<SystemComponentState>&&);
    virtual void process(std::unique_ptr<SystemComponentState_Request>&&);

    virtual void process(std::unique_ptr<Cmd>&&);
    virtual void process(std::unique_ptr<CmdCancel>&&);
    virtual void process(std::unique_ptr<CmdState>&&);
    virtual void process(std::unique_ptr<CmdSubscribe_Request>&&);
    virtual void process(std::unique_ptr<CmdSubscribe_Response>&&);

    virtual void process(std::unique_ptr<DeviceActivate_Request>&&);
    virtual void process(std::unique_ptr<DeviceActivate_Response>&&);
    virtual void process(std::unique_ptr<DeviceConnect_Request>&&);
    virtual void process(std::unique_ptr<DeviceConnect_Response>&&);
    virtual void process(std::unique_ptr<DeviceDisconnect_Request>&&);
    virtual void process(std::unique_ptr<DeviceDisconnect_Response>&&);
    virtual void process(std::unique_ptr<DeviceFileLoad_Request>&&);
    virtual void process(std::unique_ptr<DeviceFileLoad_Response>&&);
    virtual void process(std::unique_ptr<DeviceFileLoadCancel_Request>&&);
    virtual void process(std::unique_ptr<DeviceList_Request>&&);
    virtual void process(std::unique_ptr<DeviceList_Response>&&);
    virtual void process(std::unique_ptr<DeviceDescription_Request>&&);
    virtual void process(std::unique_ptr<DeviceDescription_Response>&&);
    virtual void process(std::unique_ptr<DeviceRegister_Request>&&);
    virtual void process(std::unique_ptr<DeviceRegister_Response>&&);
    virtual void process(std::unique_ptr<DeviceRegistered>&&);
    virtual void process(std::unique_ptr<DeviceUnRegistered>&&);
    virtual void process(std::unique_ptr<DeviceUpdate_Request>&&);
    virtual void process(std::unique_ptr<DeviceUpdate_Response>&&);
    virtual void process(std::unique_ptr<DeviceUpdated>&&);
    virtual void process(std::unique_ptr<DeviceUnRegister_Request>&&);
    virtual void process(std::unique_ptr<DeviceUnRegister_Response>&&);
    virtual void process(std::unique_ptr<DeviceState_Response>&&);
    virtual void process(std::unique_ptr<DeviceActionLog>&&);

    virtual void process(std::unique_ptr<FirmwareList_Request>&&);
    virtual void process(std::unique_ptr<FirmwareList_Response>&&);
    virtual void process(std::unique_ptr<FirmwareDescription_Request>&&);
    virtual void process(std::unique_ptr<FirmwareDescription_Response>&&);
    virtual void process(std::unique_ptr<FirmwareRegister_Request>&&);
    virtual void process(std::unique_ptr<FirmwareRegister_Response>&&);
    virtual void process(std::unique_ptr<FirmwareRegistered>&&);

    virtual void process(std::unique_ptr<Channel_Request>&&);
    virtual void process(std::unique_ptr<Channel_Response>&&);
    virtual void process(std::unique_ptr<ChannelState_Response>&&);

    virtual void process(std::unique_ptr<TmParamSubscribe_Request>&&);
    virtual void process(std::unique_ptr<TmParamSubscribe_Response>&&);
    virtual void process(std::unique_ptr<TmParamList>&&);

    virtual void process(std::unique_ptr<ProtocolList_Request>&&);
    virtual void process(std::unique_ptr<ProtocolList_Response>&&);
    virtual void process(std::unique_ptr<ProtocolDescription_Request>&&);
    virtual void process(std::unique_ptr<ProtocolDescription_Response>&&);
    virtual void process(std::unique_ptr<ProtocolForDevice_Request>&&);
    virtual void process(std::unique_ptr<ProtocolForDevice_Response>&&);
    virtual void process(std::unique_ptr<ProtocolForDeviceRegister_Request>&&);
    virtual void process(std::unique_ptr<ProtocolForDeviceRegister_Response>&&);
};

}
}

