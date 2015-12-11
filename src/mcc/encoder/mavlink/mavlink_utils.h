/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#pragma once

#include <cstdint>
#include <vector>

#include "bmcl/MemWriter.h"

#include "mavlink/common/mavlink.h"

#include "mcc/misc/Route.h"

namespace mcc
{
namespace mavlink
{
const char * ackTypeToString(uint8_t type);
const char * frameToString(MAV_FRAME frame);
const char * modeToString(MAV_MODE mode);
const char * resultToString(MAV_RESULT result);

/*!
 * Закодировать Мавлинк-пакет в байтовый массив
 */
std::vector<uint8_t> encodeMavlinkPacket(uint8_t messageId, const uint8_t *start, uint8_t size);
/*!
 * Отправить Мавлинк-пакет
 */
void sendMavlinkPacket(mcc::misc::Sender<std::vector<uint8_t>> & sender, uint8_t messageId, bmcl::MemWriter args);

void sendMissionItem(mcc::misc::Sender<std::vector<uint8_t>> & sender, uint8_t targetSystem, uint8_t targetComponent,
                     uint16_t waypointIndex, MAV_FRAME frame, mcc::misc::PointCoordinates point, bool isNextWaypoint,
                     uint8_t autocontinue = 1, MAV_CMD cmd = MAV_CMD_NAV_WAYPOINT, float param1 = 0.f,
                     float param2 = 0.f, float param3 = 0.f, float param4 = 0.f);

/*!
 * Отправить Heartbeat пакет Mavlink
 */
void sendMavlinkHeartbeat(mcc::misc::Sender<std::vector<uint8_t>> & sender);

void encodeMavlinkHeartbeat(bmcl::MemWriter & writer);
}
}
