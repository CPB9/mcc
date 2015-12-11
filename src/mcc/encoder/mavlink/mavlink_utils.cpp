/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "bmcl/Panic.h"
#include "bmcl/MemWriter.h"

#include <QDebug>

#include "mcc/encoder/mavlink/Mavlink.h"

#include "mavlink_utils.h"

using mcc::encoder::mavlink::MavlinkCoder;

using Sender = mcc::misc::Sender<std::vector<uint8_t>>;

namespace mcc
{
namespace mavlink
{
const char * ackTypeToString(uint8_t type)
{
    switch (type)
    {
    case 0:
        return "Success";
    case MAV_MISSION_UNSUPPORTED_FRAME:
        return "Coordinate frame unsupported";
    case MAV_MISSION_UNSUPPORTED:
        return "Unsupported command";
    case MAV_MISSION_NO_SPACE:
        return "Mission count exceeds storage";
    case MAV_MISSION_INVALID:
        return "A specified parameter was invalid (parameter not set)";
    case MAV_MISSION_INVALID_PARAM1:
        return "A specified parameter was invalid (param1)";
    case MAV_MISSION_INVALID_PARAM2:
        return "A specified parameter was invalid (param2)";
    case MAV_MISSION_INVALID_PARAM3:
        return "A specified parameter was invalid (param3)";
    case MAV_MISSION_INVALID_PARAM4:
        return "A specified parameter was invalid (param4)";
    case MAV_MISSION_INVALID_PARAM5_X:
        return "A specified parameter was invalid (param5/x)";
    case MAV_MISSION_INVALID_PARAM6_Y:
        return "A specified parameter was invalid (param6/y)";
    case MAV_MISSION_INVALID_PARAM7:
        return "A specified parameter was invalid (param7/z)";
    case MAV_MISSION_INVALID_SEQUENCE:
        return "Mission received out of sequence";
    case MAV_MISSION_DENIED:
        return "UAS not accepting missions";
    case MAV_MISSION_ERROR:
    default:
        return "Unspecified error";
    }
}
const char * frameToString(MAV_FRAME frame)
{
    switch (frame)
    {
    case MAV_FRAME_GLOBAL:
        return "Global coordinate frame, WGS84 coordinate system. First value / x: latitude, second value / y: longitude, third value / z: positive altitude over mean sea level (MSL)";
    case MAV_FRAME_LOCAL_NED:
        return "Local coordinate frame, Z-up (x: north, y: east, z: down).";
    case MAV_FRAME_MISSION:
        return "NOT a coordinate frame, indicates a mission command.";
    case MAV_FRAME_GLOBAL_RELATIVE_ALT:
        return "Global coordinate frame, WGS84 coordinate system, relative altitude over ground with respect to the home position. First value / x: latitude, second value / y: longitude, third value / z: positive altitude with 0 being at the altitude of the home location.";
    case MAV_FRAME_LOCAL_ENU:
        return "Local coordinate frame, Z-down (x: east, y: north, z: up)";
    case MAV_FRAME_GLOBAL_INT:
        return "Global coordinate frame, WGS84 coordinate system. First value / x: latitude in degrees*1.0e-7, second value / y: longitude in degrees*1.0e-7, third value / z: positive altitude over mean sea level (MSL)";
    case MAV_FRAME_GLOBAL_RELATIVE_ALT_INT:
        return "Global coordinate frame, WGS84 coordinate system, relative altitude over ground with respect to the home position. First value / x: latitude in degrees*10e-7, second value / y: longitude in degrees*10e-7, third value / z: positive altitude with 0 being at the altitude of the home location.";
    case MAV_FRAME_LOCAL_OFFSET_NED:
        return "Offset to the current local frame. Anything expressed in this frame should be added to the current local frame position.";
    case MAV_FRAME_BODY_NED:
        return "Setpoint in body NED frame. This makes sense if all position control is externalized - e.g. useful to command 2 m/s^2 acceleration to the right.";
    case MAV_FRAME_BODY_OFFSET_NED:
        return "Offset in body NED frame. This makes sense if adding setpoints to the current flight path, to avoid an obstacle - e.g. useful to command 2 m/s^2 acceleration to the east.";
    case MAV_FRAME_GLOBAL_TERRAIN_ALT:
        return "Global coordinate frame with above terrain level altitude. WGS84 coordinate system, relative altitude over terrain with respect to the waypoint coordinate. First value / x: latitude in degrees, second value / y: longitude in degrees, third value / z: positive altitude in meters with 0 being at ground level in terrain model.";
    case MAV_FRAME_GLOBAL_TERRAIN_ALT_INT:
        return "Global coordinate frame with above terrain level altitude. WGS84 coordinate system, relative altitude over terrain with respect to the waypoint coordinate. First value / x: latitude in degrees*10e-7, second value / y: longitude in degrees*10e-7, third value / z: positive altitude in meters with 0 being at ground level in terrain model.";
    default:
        bmcl::panic("not implemented");
    }
}
const char * modeToString(MAV_MODE mode)
{
    switch (mode)
    {
    case MAV_MODE_PREFLIGHT:
        return "MAV_MODE_PREFLIGHT System is not ready to fly, booting, calibrating, etc. No flag is set.";
    case MAV_MODE_MANUAL_DISARMED:
        return "MAV_MODE_MANUAL_DISARMED System is allowed to be active, under manual (RC) control, no stabilization";
    case MAV_MODE_TEST_DISARMED:
        return "MAV_MODE_TEST_DISARMED UNDEFINED mode. This solely depends on the autopilot - use with caution, intended for developers only.";
    case MAV_MODE_STABILIZE_DISARMED:
        return "MAV_MODE_STABILIZE_DISARMED System is allowed to be active, under assisted RC control.";
    case MAV_MODE_GUIDED_DISARMED:
        return "MAV_MODE_GUIDED_DISARMED System is allowed to be active, under autonomous control, manual setpoint";
    case MAV_MODE_AUTO_DISARMED:
        return "MAV_MODE_AUTO_DISARMED System is allowed to be active, under autonomous control and navigation (the trajectory is decided onboard and not pre-programmed by MISSIONs)";
    case MAV_MODE_MANUAL_ARMED:
        return "MAV_MODE_MANUAL_ARMED System is allowed to be active, under manual (RC) control, no stabilization";
    case MAV_MODE_TEST_ARMED:
        return "MAV_MODE_TEST_ARMED UNDEFINED mode. This solely depends on the autopilot - use with caution, intended for developers only.";
    case MAV_MODE_STABILIZE_ARMED:
        return "MAV_MODE_STABILIZE_ARMED System is allowed to be active, under assisted RC control.";
    case MAV_MODE_GUIDED_ARMED:
        return "MAV_MODE_GUIDED_ARMED System is allowed to be active, under autonomous control, manual setpoint";
    case MAV_MODE_AUTO_ARMED:
        return "MAV_MODE_AUTO_ARMED System is allowed to be active, under autonomous control and navigation (the trajectory is decided onboard and not pre-programmed by MISSIONs)";
    default:
        bmcl::panic("not implemented");
    }
}
const char * resultToString(MAV_RESULT result)
{
    switch (result)
    {
    case MAV_RESULT_ACCEPTED:
        return "Command ACCEPTED and EXECUTED";
    case MAV_RESULT_TEMPORARILY_REJECTED:
        return "Command TEMPORARY REJECTED/DENIED";
    case MAV_RESULT_DENIED:
        return "Command PERMANENTLY DENIED";
    case MAV_RESULT_UNSUPPORTED:
        return "Command UNKNOWN/UNSUPPORTED";
    case MAV_RESULT_FAILED:
        return "Command executed, but failed";
    default:
        bmcl::panic("not implemented");
    }
}

std::vector<uint8_t> encodeMavlinkPacket(uint8_t messageId, const uint8_t *start, uint8_t size)
{
    static uint8_t counter = 0;

    BMCL_ASSERT_MSG(size == MavlinkCoder::messageLengthByMessageId[messageId], "invalid message size");
    std::vector<uint8_t> result;
    result.resize(size + MAVLINK_CORE_HEADER_LEN + 3);
    bmcl::MemWriter packetWriter(result.data(), result.size());

    packetWriter.writeUint8(0xFE);
    packetWriter.writeUint8(static_cast<uint8_t>(size)); //Payload length
    packetWriter.writeUint8(counter++); // Packet sequence
    packetWriter.writeUint8(0xff); // System ID
    packetWriter.writeUint8(0); // Component ID
    packetWriter.writeUint8(messageId); // Message ID
    packetWriter.write(start, size); //Data
    packetWriter.writeUint16Le(MavlinkCoder::computeCrc(messageId, packetWriter.start() + 1,
                                                        static_cast<uint16_t>(packetWriter.sizeUsed() - 1)));
    BMCL_ASSERT(packetWriter.isFull());
    return result;
}


void sendMavlinkPacket(Sender & sender, uint8_t messageId, bmcl::MemWriter args)
{
    qDebug() << "Sending Mavlink message" << messageId;
    sender.send(encodeMavlinkPacket(messageId, args.start(), static_cast<uint8_t>(args.sizeUsed())));
}

void sendMissionItem(Sender & sender, uint8_t targetSystem, uint8_t targetComponent, uint16_t waypointIndex,
                     MAV_FRAME frame, mcc::misc::PointCoordinates point, bool isNextWaypoint, uint8_t autocontinue,
                     MAV_CMD cmd, float param1, float param2, float param3, float param4)
{
    uint8_t buf[MAVLINK_MSG_ID_MISSION_ITEM_LEN];
    bmcl::MemWriter writer(buf);
    writer.writeFloat32Le(param1); // Hold time in decimal seconds. (ignored by fixed wing, time to stay at MISSION for rotary wing)
    writer.writeFloat32Le(param2); // Acceptance radius in meters (if the sphere with this radius is hit, the MISSION counts as reached)
    writer.writeFloat32Le(param3); // 0 to pass through the WP, if > 0 radius in meters to pass by WP. Positive value for clockwise orbit, negative value for counter-clockwise orbit. Allows trajectory control.
    writer.writeFloat32Le(param4); // Desired yaw angle at MISSION (rotary wing)
    writer.writeFloat32Le(static_cast<float>(point.latitudeDeg()));
    writer.writeFloat32Le(static_cast<float>(point.longitudeDeg()));
    writer.writeFloat32Le(static_cast<float>(point.altitude()));
    writer.writeUint16(waypointIndex);
    writer.writeUint16(cmd);
    writer.writeUint8(targetSystem);
    writer.writeUint8(targetComponent);
    writer.writeUint8(frame);
    writer.writeUint8(static_cast<uint8_t>(isNextWaypoint ? 1 : 0));
    writer.writeUint8(autocontinue); // 2 для guided режима в ARDUPilotMega? так написано в QGC
    sendMavlinkPacket(sender, MAVLINK_MSG_ID_MISSION_ITEM, writer);
}

void encodeMavlinkHeartbeat(bmcl::MemWriter & writer)
{
    writer.writeUint32Le(0);
    writer.writeUint8(MAV_TYPE_GCS);
    writer.writeUint8(MAV_AUTOPILOT_INVALID);
    writer.writeUint8(MAV_MODE_STABILIZE_ARMED);
    writer.writeUint8(MAV_STATE_ACTIVE);
    writer.writeUint8(MAVLINK_VERSION);
    encodeMavlinkPacket(MAVLINK_MSG_ID_HEARTBEAT, writer.start(), static_cast<uint8_t>(writer.sizeUsed()));
}
}
}