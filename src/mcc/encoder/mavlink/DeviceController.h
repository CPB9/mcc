/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#pragma once
#include <string>
#include <limits>
#include <array>

#include "mcc/core/decode/Variant.h"
#include "mcc/core/decode/Registry.h"
#include "bmcl/MemWriter.h"
#include "bmcl/Result.h"
#include "bmcl/Option.h"

#include "mcc/messages/Cmd.h"
#include "mcc/misc/Device.h"
#include "mcc/misc/Cmd.h"
#include "mcc/misc/CommonMath.h"
#include "mcc/messages/Deaclarations.h"
#include "mcc/modeling/flying_device.h"
#include "mcc/encoder/core/DeviceTask.h"
#include "mcc/encoder/mavlink/RouteController.h"

typedef enum { Roll = 0, Pitch, Throttle, Heading, _4, _5, _6, _7, Count, First = Roll, Last = _7 } RcChannels;

const uint_fast16_t rcChannelMin = 1000;
const uint_fast16_t rcChannelMax = 2000;
const uint_fast16_t rcChannelRange = rcChannelMax - rcChannelMin;

const double throttleMinPer = 0.;
const double throttleMaxPer = 100.;
const double throttleRangePer = throttleMaxPer - throttleMinPer;
const double throttleUpdateScale = .1;

namespace mcc { namespace decode { class Component; }}

namespace mcc {
namespace encoder {
namespace mavlink {

using ui16_lim = std::numeric_limits<uint16_t>;

/**!
 * Класс не поддерживает параллельную работу в нескольких потоках!
 */
class DeviceController : public mcc::encoder::core::DeviceTask
{
public:
    DeviceController(const std::string& name, const std::string& device_name, const mcc::messages::MessageSender& out, std::size_t triesCount);
    virtual ~DeviceController();
    bool pre() override;
    void tick() override;
    void push(std::unique_ptr<mcc::messages::Cmd>&& cmd);
    void cancel(std::unique_ptr<mcc::messages::CmdCancel>&& cancel);
    std::size_t cmdsInQueue() const;
    std::size_t cmdsProcessed() const;

private:
    bmcl::Option<std::string> execute_(const mcc::messages::Cmd& cmd);

    std::atomic<std::size_t> _cmdsInQueue;
    std::atomic<std::size_t> _cmdsProcessed;

    std::deque<std::unique_ptr<mcc::messages::Cmd>> _cmds;
    std::shared_ptr<decode::Component> _pixhawkComponent;

    std::array<uint16_t, RcChannels::Count> _rcChannelsOverride;
    std::array<double, RcChannels::Count> _rcChannelsScale;

    uint8_t _targetSystem, _targetComponent;

    RouteController _routeController;
    std::unique_ptr<modeling::FlyingDeviceModesFsm> _modesFsm;

    void provideOldStyleTm(uint16_t messageId, const mcc::decode::MapVariant &value);
    void sendCommand(uint16_t cmdId, uint8_t targetSystem, uint8_t targetComponent, uint8_t confirmation = 0,
                     float param1 = 0.f, float param2 = 0.f, float param3 = 0.f, float param4 = 0.f, float param5 = 0.f,
                     float param6 = 0.f, float param7 = 0.f);
    void sendRcChannelsOverride();
    void setThrottle(double throttlePer);
    void sendCommandStateEvent(const mcc::messages::Cmd& cmd, messages::CmdState::Value state, const std::string & reason = std::string());
    void setHeading(double headingDeg);
    void updateRcChannelValue(uint8_t channelNum, uint16_t value);
    void setRoll(double rollDeg);
    void setPitch(double pitchDeg);
    void incThrottle(double throttle);
    void decThrottle(double throttlePer);
    void setMode(MAV_MODE mode);
    static modeling::FlyingDeviceMode mavlinkModeToFlyingDeviceMode(MAV_MODE_FLAG mavModeFlag);
};

typedef std::shared_ptr<DeviceController> DeviceCommanderPtr;

}
}
}

