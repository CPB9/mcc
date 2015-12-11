/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include <chrono>

#include <QDebug>
#include <QtMath>

#include "mavlink/common/mavlink.h"

#include "mcc/misc/CommonMath.h"
#include "mcc/misc/Crc.h"
#include "mcc/misc/TmParam.h"
#include "mcc/messages/MessageSender.h"
#include "mcc/messages/Cmd.h"
#include "mcc/messages/Tm.h"
#include "mcc/encoder/core/ExchangeHelper.h"
#include "mcc/encoder/mavlink/mavlink_utils.h"
#include "mcc/encoder/mavlink/Mavlink.h"
#include "mcc/encoder/mavlink/DeviceController.h"

#include "mcc/core/decode/Registry.h"
#include "mcc/core/decode/Sqlite3RegistryProviderSingleton.h"

using mcc::misc::limit;

using mcc::misc::latitudeMinDeg;
using mcc::misc::latitudeMaxDeg;
using mcc::misc::latitudeRangeDeg;

using mcc::misc::longitudeMinDeg;
using mcc::misc::longitudeMaxDeg;
using mcc::misc::longitudeRangeDeg;

using mcc::misc::rollMinDeg;
using mcc::misc::rollMaxDeg;
using mcc::misc::rollRangeDeg;

using mcc::misc::pitchMinDeg;
using mcc::misc::pitchMaxDeg;
using mcc::misc::pitchRangeDeg;

using mcc::misc::headingMinDeg;
using mcc::misc::headingMaxDeg;
using mcc::misc::headingRangeDeg;

using mcc::misc::ChannelError;
using mcc::misc::ParamValueList;

using mcc::messages::TmParams;

using mcc::decode::Component;
using mcc::decode::OptionPtr;
using mcc::decode::Command;
using mcc::decode::MapVariant;
using mcc::decode::Message;
using mcc::decode::ValueOrError;
using mcc::decode::Name;

using mcc::mavlink::modeToString;
using mcc::mavlink::resultToString;
using mcc::mavlink::encodeMavlinkHeartbeat;
using mcc::mavlink::sendMavlinkPacket;

using M = mcc::modeling::FlyingDeviceMode;
using T = mcc::modeling::FlyingDeviceModesFsmTransition;
using Nav = mcc::modeling::NavigationCommands;

namespace mcc {
namespace encoder {
namespace mavlink {

using clock = std::chrono::high_resolution_clock;
using time = clock::time_point;

DeviceController::DeviceController(const std::string& name, const std::string& device_name, const mcc::messages::MessageSender& out, std::size_t triesCount)
        : mcc::encoder::core::DeviceTask(name, device_name, out, triesCount)
        , _targetSystem(1)
        , _targetComponent(0)
        , _routeController(_innerChannel.sender, _out, _device_name)
{
    _rcChannelsOverride = { { 0 } };
    _rcChannelsScale = { { .2 } };
    _rcChannelsScale[RcChannels::Throttle] = 1.;
    _cmdsInQueue = 0;
    _cmdsProcessed = 0;
    _pixhawkComponent = mcc::decode::findDecodeComponentOrFail("mavlink.Pixhawk");
    _modesFsm.reset(new modeling::FlyingDeviceModesFsm(M::EngineReady, std::vector<T>({
            T(M::EngineReady, Nav::takeOff(), M::TakingOff, [&]() {
                setMode(MAV_MODE_GUIDED_DISARMED);
                sendCommand(400, 1, 1, 0, 1);
            }),
            T(M::EngineReady, Nav::flyRoute(), M::RouteFlying, [&]() {
                sendCommand(MAV_CMD_COMPONENT_ARM_DISARM, 1, 0, 0, 1);
                sendCommand(MAV_CMD_MISSION_START, _targetSystem, _targetComponent, 0, _routeController.route().size() - 1);
            }),
            T(M::TakingOff, Nav::flyRoute(), M::RouteFlying, [&]() {
                setMode(MAV_MODE_AUTO_ARMED);
            }),
            T(M::RouteFlying, Nav::guidedFlying(), M::GuidedFlying, [&]() {
                setMode(MAV_MODE_GUIDED_ARMED);
            }),
                                                                                      })));
}

DeviceController::~DeviceController()
{
    finish(true);
}

void DeviceController::push(std::unique_ptr<mcc::messages::Cmd>&& cmd)
{
    ++_cmdsInQueue;
    _out->send<mcc::messages::CmdState>(*cmd, mcc::messages::CmdState::Value::WaitingInDeliveryQueue);
    std::lock_guard<std::mutex> lock(_mutex);
    _cmds.push_back(std::move(cmd));
    _wakeup.notify_one();
}

void DeviceController::cancel(std::unique_ptr<mcc::messages::CmdCancel>&& cancel)
{
    std::lock_guard<std::mutex> lock(_mutex);
    auto i = std::find_if(_cmds.begin(), _cmds.end(), [&cancel](const std::unique_ptr<mcc::messages::Cmd>& cmd){return cmd->cmdId() == cancel->cmdId(); });
    if (i == _cmds.end())
        return;

    _out->send<mcc::messages::CmdState>(*i->get(), mcc::messages::CmdState::Value::Failed, "canceled by user");
    _cmds.erase(i);
}

std::size_t DeviceController::cmdsInQueue() const
{
    return _cmdsInQueue;
}

std::size_t DeviceController::cmdsProcessed() const
{
    return _cmdsProcessed;
}

bool DeviceController::pre()
{
    qDebug() << "Initializing Mavlink device connection";

    // Отправить Хатбит
    std::vector<uint8_t> vec;
    vec.resize(sizeof(uint32_t) + sizeof(uint8_t) * 5);
    bmcl::MemWriter writer(vec.data(), vec.size());
    encodeMavlinkHeartbeat(writer);
    _innerChannel.sender.send(vec);

    /*uint8_t p1[] = { 0x0d, 0x0d, 0x0d };
    channel->send(p1, sizeof(p1));

    uint8_t shScriptRun[] = "sh /etc/init.d/rc.usb\n";
    channel->send(shScriptRun, sizeof(shScriptRun));

    uint8_t p3[] = { 0x0d, 0x0d, 0x0d, 0xa6 };
    channel->send(shScriptRun, sizeof(p3));*/
    return true;
}

void DeviceController::tick()
{
    // Логика работы с устройством

    _routeController.tick();

    // Логика работы с обменом

    auto r = _innerChannel.reciever.tryRecvFor(std::chrono::milliseconds(100));

    if (r.isErr() && r.takeErr() == ChannelError::Closed)
    {
        qDebug() << "channel closed";
        return;
    }

    if (r.isOk())
    {
        auto tm = r.take();
        auto t = MavlinkCoder::decode(tm.data(), tm.size());
        if (t.isErr())
        {
            qDebug() << QString::fromStdString(toString(t.takeErr()));
        }
        else
        {
            MavlinkMessage message = t.take();
            const Message & messageModel = *_pixhawkComponent->messageById(message._messageId);
            //qDebug() << "Received message:" << QString::fromStdString(messageModel.name());
            bmcl::MemReader reader(message._data.data(), message._data.size());
            ValueOrError value = messageModel.deserialize(reader);
            auto & messageData = value.value()->asMap();
            _routeController.handleTmMessage(message._messageId, messageData);
            provideOldStyleTm(message._messageId, messageData);
            switch (message._messageId)
            {
            case MAVLINK_MSG_ID_HEARTBEAT:
                {
                    auto mode = mavlinkModeToFlyingDeviceMode(
                            static_cast<MAV_MODE_FLAG>(messageData.at("_0").asMap().at("baseMode").toUint64()));
                    if (_modesFsm->mode() != mode)
                        _modesFsm->mode(mode);
                }
                break;
            default:
                break;
            }
        }
    }

    static time last = clock::now();
    time now = clock::now();
    // Обновляем только в режиме ручного управления
    if (_modesFsm->mode() == M::GuidedFlying
        && (now - last) > std::chrono::milliseconds(50))
    {
        last = now;
        sendRcChannelsOverride();
    }

    if (_cmdsInQueue > 0)
    {
        _mutex.lock();
        if (!_cmds.empty())
        {
            auto cmd = std::move(_cmds.front());
            _cmds.pop_front();
            _mutex.unlock();
            execute_(*cmd);
            --_cmdsInQueue;
            ++_cmdsProcessed;
        }
        else
        {
            _mutex.unlock();
        }
    }
}

bmcl::Option<std::string> DeviceController::execute_(const mcc::messages::Cmd& cmd)
{
    if (_routeController.handleCommand(cmd))
        return bmcl::None;
    const std::string& trait = cmd.trait();
    const std::string& command = cmd.command();
    //Q_ASSERT(cmd.params().size() == 7);
    modeling::Command mCmd(trait + "." + command);
    if (_modesFsm->isValidAction(mCmd))
    {
        if (_modesFsm->activate(mCmd))
        {

        }
        else
        {
            sendCommandStateEvent(cmd, messages::CmdState::Failed, "Недопустимая команда для текущего режима");
            return bmcl::Option<std::string>("Недопустимая команда для текущего режима");
        }
    }
    /*if (trait == "Navigation")
    {
        if (_modesFsm->isValidAction())
        if (command == "startEngine")
        {
            sendCommand(400, 1, 1, 0, 1); // arm
        }
        else if (command == "takeOff")
        {
            setThrottle(25.);
        }
        else if (command == "land")
        {
            sendCommand(400, 1, 1, 0, 0); // disarm
        }
        return bmcl::None;
    }*/
    else if (trait == "Navigation.Joystick")
    {
        if (command == "setHeading")
        {
            setHeading(cmd.params().at(0).toDouble());
        }
        else if (command == "setRoll")
        {
            setRoll(cmd.params().at(0).toDouble());
        }
        else if (command == "setPitch")
        {
            setPitch(cmd.params().at(0).toDouble());
        }
        else if (command == "setThrottle")
        {
            setThrottle(cmd.params().at(0).toDouble());
        }
        else if (command == "incThrottle")
        {
            incThrottle(cmd.params().at(0).toDouble());
        }
        else if (command == "decThrottle")
        {
            decThrottle(cmd.params().at(0).toDouble());
        }
    }
    sendCommandStateEvent(cmd, messages::CmdState::AcknowledgeReceived);
    return bmcl::None;// toString(mcc::encoder::core::DecodeError::EncodeUnknownCommand);
}

void DeviceController::sendCommandStateEvent(const mcc::messages::Cmd& cmd, messages::CmdState::Value state, const std::string & reason)
{
    std::string cmdName = cmd.name();
    if (state == messages::CmdState::Value::Failed)
        qDebug() << QString::fromStdString(cmdName) << " не выполнена: " << QString::fromStdString(reason);
    else if (state == messages::CmdState::Value::AcknowledgeReceived)
        qDebug() << QString::fromStdString(cmdName) << " выполнена";

    _out->send<mcc::messages::CmdState>(cmd, state, reason);
}


void DeviceController::sendCommand(uint16_t cmdId, uint8_t targetSystem, uint8_t targetComponent, uint8_t confirmation,
                                   float param1, float param2, float param3, float param4, float param5, float param6,
                                   float param7)
{
    static uint8_t messageId = MAVLINK_MSG_ID_COMMAND_LONG;

    MapVariant cmdArgs;
    cmdArgs.with("param1", param1)
            .with("param2", param2)
            .with("param3", param3)
            .with("param4", param4)
            .with("param5", param5)
            .with("param6", param6)
            .with("param7", param7)
            .with("command", cmdId)
            .with("targetSystem", targetSystem)
            .with("targetComponent", targetComponent)
            .with("confirmation", confirmation);
    const Command & cmd = *_pixhawkComponent->commandById(cmdId);
    uint8_t cmdArgsBuf[100];
    bmcl::MemWriter argsWriter(cmdArgsBuf);
    auto result = cmd.serialize(cmdArgs, argsWriter);
    BMCL_ASSERT_MSG(result.isNone(), "can't serialize command message arguments");
    BMCL_ASSERT_MSG(argsWriter.sizeUsed() == MavlinkCoder::messageLengthByMessageId[messageId], "invalid message size");
    sendMavlinkPacket(_innerChannel.sender, messageId, argsWriter);
}

void DeviceController::provideOldStyleTm(uint16_t messageId, const mcc::decode::MapVariant &value)
{
    switch (messageId)
    {
    case MAVLINK_MSG_ID_SYS_STATUS:
        {
            const MapVariant & data = value.at("_1").asMap();
            uint8_t batteryLevel = static_cast<uint8_t>(data.at("batteryRemaining").toInt64());
            if (batteryLevel > 100)
                batteryLevel = 0;
            TmParams params = {
                    {"Device", "batteryLevel", batteryLevel},
            };
            std::string deviceTrait = "Device";
            params.emplace_back(deviceTrait, "signalLevel", 100);
            _out->send<mcc::messages::TmParamList>(_device_name, std::move(params));
        }
        break;
    case MAVLINK_MSG_ID_ATTITUDE:
        {
            const MapVariant & data = value.at("_30").asMap();
            double pitchDeg = data.at("pitch").toFloat64(), rollDeg = data.at("roll").toFloat64(),
                    yawDeg = data.at("yaw").toFloat64();
            std::string navMotionTraitName = "Navigation.Motion";
            TmParams params = {
                    {navMotionTraitName, "pitch", misc::radiansToDegrees(pitchDeg)},
                    {navMotionTraitName, "heading", misc::radiansToDegrees(yawDeg)},
                    {navMotionTraitName, "roll", misc::radiansToDegrees(rollDeg)},
                    {navMotionTraitName, "throttle", _rcChannelsOverride[RcChannels::Throttle]}
            };
            _out->send<mcc::messages::TmParamList>(_device_name, std::move(params));
        }
        break;
    case MAVLINK_MSG_ID_GLOBAL_POSITION_INT:
        {
            const MapVariant & data = value.at("_33").asMap();
            double latitudeDeg = data.at("lat").toInt64() / 10000000., longitudeDeg = data.at("lon").toInt64() / 10000000.;
            double altitudeM = data.at("alt").toInt64() / 1000.;
            std::string navMotionTraitName = "Navigation.Motion";
            TmParams params = {
                    {navMotionTraitName, "latitude", latitudeDeg},
                    {navMotionTraitName, "longitude", longitudeDeg},
                    {navMotionTraitName, "altitude", altitudeM}
            };
            _out->send<mcc::messages::TmParamList>(_device_name, std::move(params));
        }
        break;
    case MAVLINK_MSG_ID_COMMAND_ACK:
        {
            auto & data = value.at("_77").asMap();
            uint16_t cmd = static_cast<uint16_t>(data.at("command").toUint64());
            MAV_RESULT result = static_cast<MAV_RESULT>(data.at("result").toUint64());
            qDebug() << "Command" << cmd << resultToString(result);
        }
        break;
    default:
        break;
    }
}

void DeviceController::sendRcChannelsOverride()
{
    uint8_t buf[sizeof(uint16_t) * RcChannels::Count + sizeof(uint8_t) * 2];
    bmcl::MemWriter writer(buf);
    // Нет смысла слать пакеты без изменений
    bool hasChanges = false;
    for (size_t i = 0; i < sizeof(_rcChannelsOverride) / sizeof(_rcChannelsOverride[0]); i++)
    {
        auto & rcChannelValue = _rcChannelsOverride[i];
        if (rcChannelValue != 0)
            hasChanges = true;
        writer.writeUint16Le(rcChannelValue);
    }
    if (hasChanges)
    {
        writer.writeUint8(_targetSystem);
        writer.writeUint8(_targetComponent);
        sendMavlinkPacket(_innerChannel.sender, MAVLINK_MSG_ID_RC_CHANNELS_OVERRIDE, writer);
    }
}

void DeviceController::setThrottle(double throttlePer)
{
    BMCL_ASSERT(throttlePer >= throttleMinPer && throttlePer <= throttleMaxPer);
    updateRcChannelValue(RcChannels::Throttle, static_cast<uint16_t>(rcChannelMin + rcChannelRange * throttlePer / throttleRangePer));
}

void DeviceController::setHeading(double headingDeg)
{
    BMCL_ASSERT(headingDeg >= headingMinDeg && headingDeg <= headingMaxDeg);
    updateRcChannelValue(RcChannels::Heading, static_cast<uint16_t>(rcChannelMin + rcChannelRange * (headingDeg + headingRangeDeg / 2.) / headingRangeDeg));
}

void DeviceController::updateRcChannelValue(uint8_t channelNum, uint16_t value)
{
    BMCL_ASSERT(value >= rcChannelMin && value <= rcChannelMax);
    _rcChannelsOverride[channelNum] = value;
}

void DeviceController::setRoll(double rollDeg)
{
    BMCL_ASSERT(rollDeg >= rollMinDeg && rollDeg <= rollMaxDeg);
    updateRcChannelValue(RcChannels::Roll, static_cast<uint16_t>(rcChannelMin + rcChannelRange * (rollDeg + rollRangeDeg / 2.) / rollRangeDeg));
}

void DeviceController::setPitch(double pitchDeg)
{
    BMCL_ASSERT(pitchDeg >= pitchMinDeg && pitchDeg <= pitchMaxDeg);
    updateRcChannelValue(RcChannels::Pitch, static_cast<uint16_t>(rcChannelMin + rcChannelRange * (pitchDeg + pitchRangeDeg / 2.) / pitchRangeDeg));
}

void DeviceController::incThrottle(double throttlePer)
{
    BMCL_ASSERT(throttlePer >= throttleMinPer && throttlePer <= throttleMaxPer);
    double currentThrottlePer = (_rcChannelsOverride[RcChannels::Throttle] - rcChannelMin) * throttleUpdateScale;
    setThrottle(limit(currentThrottlePer + 10. * throttlePer / throttleRangePer, throttleMinPer, throttleMaxPer));
}

void DeviceController::decThrottle(double throttlePer)
{
    BMCL_ASSERT(throttlePer >= throttleMinPer && throttlePer <= throttleMaxPer);
    double currentThrottlePer = (_rcChannelsOverride[RcChannels::Throttle] - rcChannelMin) * throttleUpdateScale;
    setThrottle(limit(currentThrottlePer - 10. * throttlePer / throttleRangePer, throttleMinPer, throttleMaxPer));
}

void DeviceController::setMode(MAV_MODE mode)
{
    qDebug() << "Setting mode" << modeToString(mode);
    uint8_t buf[sizeof(uint32_t) + 2 * sizeof(uint8_t)];
    bmcl::MemWriter writer(buf);
    writer.writeInt32Le(0);
    writer.writeUint8(_targetSystem);
    writer.writeUint8(mode);
    sendMavlinkPacket(_innerChannel.sender, MAVLINK_MSG_ID_SET_MODE, writer);
}

M DeviceController::mavlinkModeToFlyingDeviceMode(MAV_MODE_FLAG mavModeFlag)
{
    if (mavModeFlag & MAV_MODE_FLAG_AUTO_ENABLED)
    {
        return M::RouteFlying;
    }
    else if (mavModeFlag & MAV_MODE_FLAG_GUIDED_ENABLED)
    {
        return M::GuidedFlying;
    }
    else
    {
        return M::EngineReady;
    }
}
}
}
}
