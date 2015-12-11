/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#pragma once
#include <memory.h>
#include "mcc/misc/Channel.h"

namespace mcc{
namespace messages{

    typedef uint64_t MessageId;

    class Message;
    class MessageTo;
    class Response;
    class SystemState;
    class SystemState_Request;
    class SystemComponentState;
    class SystemComponentState_Request;
    class Cmd;
    class CmdCancel;
    class CmdState;
    class CmdSubscribe_Request;
    class CmdSubscribe_Response;
    class DeviceActivate_Request;
    class DeviceActivate_Response;
    class DeviceConnect_Request;
    class DeviceConnect_Response;
    class DeviceDisconnect_Request;
    class DeviceDisconnect_Response;
    class DeviceFileLoad_Request;
    class DeviceFileLoad_Response;
    class DeviceFileLoadCancel_Request;
    class DeviceList_Request;
    class DeviceList_Response;
    class DeviceDescription_Request;
    class DeviceDescription_Response;
    class DeviceRegister_Request;
    class DeviceRegister_Response;
    class DeviceRegistered;
    class DeviceUnRegistered;
    class DeviceUpdate_Request;
    class DeviceUpdate_Response;
    class DeviceUpdated;
    class DeviceUnRegister_Request;
    class DeviceUnRegister_Response;
    class DeviceState_Response;
    class DeviceActionLog;
    class FirmwareList_Request;
    class FirmwareList_Response;
    class FirmwareDescription_Request;
    class FirmwareDescription_Response;
    class FirmwareRegister_Request;
    class FirmwareRegister_Response;
    class FirmwareRegistered;
    class Channel_Request;
    class Channel_Response;
    class ChannelState_Response;
    class TmParamSubscribe_Request;
    class TmParamSubscribe_Response;
    class TmParamList;
    class ProtocolList_Request;
    class ProtocolList_Response;
    class ProtocolDescription_Request;
    class ProtocolDescription_Response;
    class ProtocolForDevice_Request;
    class ProtocolForDevice_Response;
    class ProtocolForDeviceRegister_Request;
    class ProtocolForDeviceRegister_Response;

    class MessageProcessor;
    class MessageSenderX;
    typedef std::unique_ptr<Message> MessagePtr;
    typedef mcc::misc::ChannelPtr<MessagePtr> MessageQueue;
    typedef std::shared_ptr<MessageSenderX> MessageSender;

}
}