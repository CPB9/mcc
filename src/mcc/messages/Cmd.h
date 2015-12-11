/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#pragma once
#include <string>
#include <vector>
#include <atomic>
#include "mcc/misc/TimeUtils.h"
#include "mcc/misc/NetVariant.h"
#include "mcc/misc/Cmd.h"
#include "mcc/messages/Message.h"

namespace mcc { namespace protobuf { class Cmd; } }

namespace mcc {
namespace messages {

typedef uint32_t CmdId;
//typedef std::vector<mcc::misc::NetVariant> CmdParams;

class Cmd : public MessageTo
{
public:
    Cmd(CmdId cmdId, const std::string& device, const std::string& trait, const std::string& command, const mcc::misc::CmdParams& params = mcc::misc::CmdParams())
        : MessageTo(mcc::Names::coreCmd()), _cmdId(cmdId), _device(device), _trait(trait), _command(command), _params(params)
    {
    }
    Cmd(const std::string& device, const std::string& trait, const std::string& command, const mcc::misc::CmdParams& params = mcc::misc::CmdParams())
        : Cmd(_uniqueId.fetch_add(1), device, trait, command, params)
    {
    }
    virtual ~Cmd(){}
    static std::unique_ptr<Message> deserialize(const mcc::protobuf::Cmd& cmd);
    MESSAGE_REQUIREMENTS_DECLARATIONS();

    CmdId              cmdId()   const { return _cmdId; }
    const std::string& device()  const { return _device; }
    const std::string& trait()   const { return _trait; }
    const std::string& command() const { return _command; }
    std::string        name()    const { return _device + "." + _trait + "." + _command; }
    const mcc::misc::CmdParams&   params()  const { return _params; }
    std::string paramsAsString(const std::string& delimeter) const;

protected:
    void serialize_(mcc::protobuf::MessageBody* body) const override;

private:
    static std::atomic<CmdId> _uniqueId;
    CmdId       _cmdId;
    std::string _device;
    std::string _trait;
    std::string _command;
    mcc::misc::CmdParams   _params;
};

class CmdCancel : public MessageTo
{
public:
    CmdCancel(const Cmd& cmd) : MessageTo(mcc::Names::coreCmd()), _device(cmd.device()), _cmdId(cmd.cmdId()){}
    CmdCancel(const std::string& device, CmdId cmdId) : MessageTo(mcc::Names::coreCmd()), _device(device), _cmdId(cmdId){}
    virtual ~CmdCancel(){}
    MESSAGE_REQUIREMENTS_DECLARATIONS();
    CmdId              cmdId()  const { return _cmdId; }
    const std::string& device() const { return _device; }
private:
    CmdId       _cmdId;
    std::string _device;
};

class CmdState : public MessageTo
{
public:
    enum Value
    {
        Created,
        Registered,
        WaitingForTime,
        RoutedForDelivery,
        WaitingInDeliveryQueue,
        SentToDevice,
        AcknowledgeReceived,
        //            Executed,
        Failed,
    };
    CmdState(const std::string& device, CmdId cmdId, Value state, const std::string& reason = std::string())
        : MessageTo(mcc::Names::coreCmd()), _cmdId(cmdId), _device(device), _state(state), _reason(reason)
    {
    }
    CmdState(const Cmd& cmd, Value state, const std::string& reason = std::string())
        : CmdState(cmd.device(), cmd.cmdId(), state, reason)
    {
        //нужно проставлять время формирования состояния
        _time = mcc::misc::currentDateTime();
    }

    virtual ~CmdState(){}
    MESSAGE_REQUIREMENTS_DECLARATIONS();

    static inline const char* toString(Value state)
    {
        switch (state)
        {
        case Created:               return "Команда создана";
        case Registered:            return "Команда зарегистрирована";
        case WaitingForTime:        return "Команда ожидает отправки";
        case RoutedForDelivery:     return "Команда маршрутизирована";
        case WaitingInDeliveryQueue:return "Команда ожидает в очереди";
        case SentToDevice:          return "Команда отправлена на устройство";
        case AcknowledgeReceived:   return "Получена квитанция на команду";
        case Failed:                return "Ошибка при отправке команды";
        }
        assert(false);
        return "Неизвестное состояние обработки команды";
    }

    CmdId              cmdId()   const { return _cmdId; };
    const std::string& device()  const { return _device; };
    Value              state()   const { return _state; };
    const std::string& reason()  const { return _reason; };
    const std::string& time()    const { return _time;  }
private:
    CmdId       _cmdId;
    Value       _state;
    std::string _device;
    std::string _reason;
    std::string _time;
};

class CmdSubscribe_Request : public MessageTo
{
public:
    CmdSubscribe_Request(bool isOn, const std::string& device) : MessageTo(mcc::Names::coreCmd()), _isOn(isOn), _device(device) {}
    virtual ~CmdSubscribe_Request(){}
    MESSAGE_REQUIREMENTS_DECLARATIONS();
    bool isOn() const { return _isOn; }
    const std::string& device()  const { return _device; };
private:
    bool _isOn;
    std::string _device;
};

class CmdSubscribe_Response : public Response
{
public:
    CmdSubscribe_Response(const CmdSubscribe_Request* request, const std::string& error = std::string())
        : Response(request), _isOn(request->isOn()), _device(request->device()), _error(error){}
    virtual ~CmdSubscribe_Response(){}
    MESSAGE_REQUIREMENTS_DECLARATIONS();
    bool isOn() const { return _isOn; }
    const std::string& device()  const { return _device; };
    const std::string& error()   const { return _error; };
private:
    bool _isOn;
    std::string _device;
    std::string _error;
};


}
}

