/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include <utility>

#include "bmcl/MemWriter.h"

#include "mavlink/common/mavlink.h"

#include "mcc/core/decode/Sqlite3RegistryProviderSingleton.h"

#include "mcc/messages/Tm.h"
#include "mcc/misc/tm_utils.h"

#include "mcc/encoder/mavlink/mavlink_utils.h"
#include "mcc/encoder/mavlink/Mavlink.h"
#include "mcc/encoder/mavlink/RouteController.h"

using mcc::decode::MapVariant;
using mcc::decode::Command;
using mcc::decode::Message;
using mcc::messages::TmParams;
using mcc::messages::Cmd;
using mcc::messages::CmdState;
using mcc::messages::MessageSender;
using mcc::misc::sendCommandStateEvent;

using mcc::mavlink::sendMissionItem;

using namespace mcc::mavlink;

namespace mcc {
namespace encoder {
namespace mavlink {

RouteController::RouteController(Sender & sender, MessageSender & messageSender, std::string & deviceName)
        : _sender(sender)
        , _messageSender(messageSender)
        , _deviceName(deviceName)
        , _pixhawkComponent(decode::findDecodeComponentOrFail("mavlink.Pixhawk"))
        , _targetSystem(1)
        , _targetComponent(0)
        , _waypointsState(WaypointsState::Outdated)
        , _waypointsCount(0)
        , _activeRoute(0)
        , _crc(0)
        , _mavlinkJob(Timer<>(std::chrono::milliseconds(500), false))
{
}

void RouteController::tick()
{
    provideTm();
    switch (_waypointsState)
    {
    case WaypointsState::Outdated:
        if (_sender.isOpen())
        {
            sendMissionRequestListPacket();
            toState(WaypointsState::ReadingCount);
        }
        break;
    case WaypointsState::ReadingCount:
    case WaypointsState::Reading:
    case WaypointsState::Ok:
    case WaypointsState::WritingBuffer:
    case WaypointsState::Writing:
    case WaypointsState::WritingAck:
        break;
    }
    _mavlinkJob.handle();
}

void RouteController::handleTmMessage(uint8_t messageId, MapVariant & value)
{
    switch (messageId)
    {
    case MAVLINK_MSG_ID_MISSION_COUNT:
        qDebug() << "Received waypoints count";
        if (_waypointsState == WaypointsState::ReadingCount)
        {
            const MapVariant & data = value.at("_44").asMap();
            _guidedPoint.clear();
            _route.clear();
            _waypointsCount = static_cast<uint16_t>(data.at("count").toUint64());
            qDebug() << "Waypoints count received:" << _waypointsCount;
            _route.reserve(_waypointsCount);
            if (_waypointsCount > 0)
            {
                toState(WaypointsState::Reading);
                _mavlinkJob.fireAndSchedule([&]() { requestNextWaypoint(0); });
            }
            else
            {
                toState(WaypointsState::Ok);
                _mavlinkJob.deactivate();
            }
        }
        break;
    case MAVLINK_MSG_ID_MISSION_ITEM:
        qDebug() << "Received waypoint";
        if (_waypointsState == WaypointsState::Reading)
        {
            const MapVariant & data = value.at("_39").asMap();

            uint8_t frame = static_cast<uint8_t>(data.at("frame").toUint64());
            uint16_t seq = static_cast<uint16_t>(data.at("seq").toUint64());

            qDebug() << "Received waypoint seq:" << seq;
            float latitude = data.at("x").toFloat32(), longitude = data.at("y").toFloat32(),
                    altitude = data.at("z").toFloat32();

            qDebug() << "Waypoint info has been received" << "lat:" << QString::number(latitude, 'f', 12)
                << "lon:" << QString::number(longitude, 'f', 12)
                << "alt:" << QString::number(altitude, 'f', 12) << "frame:" << frameToString((MAV_FRAME) frame);

            bool isCurrent = data.at("current").toUint64() != 0;

            RoutePoint point(latitude, longitude, altitude);
            if (seq == 0)
            {
                _guidedPoint = point;
                if (isCurrent)
                {
                    _activeRoute = 0;
                    _route.setNextWaypoint(0);
                }
            }
            else if (seq == _route.size() + 1)
            {
                _route.push_back(std::move(point));

                if (isCurrent)
                {
                    _activeRoute = 1;
                    _route.setNextWaypoint(_route.size() - 1);
                }
            }
            else
            {
                qDebug() << "Received waypoint with invalid seq";
            }

            if (_waypointsCount == _route.size() + 1)
            {
                qDebug() << "Mavlink route has been downloaded";
                toState(WaypointsState::Ok);
                _mavlinkJob.deactivate();
            }
            else
            {
                _mavlinkJob.fireAndSchedule([&]() { requestNextWaypoint(static_cast<uint16_t>(_route.size() + 1)); });
            }
        }
        else
        {
            qWarning() << "Waypoint info has been received while in non-proper state";
        }
        break;
    case MAVLINK_MSG_ID_MISSION_ITEM_REACHED:
        {
            qDebug() << "Received waypoint reached";
            uint16_t seq = static_cast<uint16_t>(value.at("_46").asMap().at("seq").toUint64());
            if (seq > 0)
                _route.setNextWaypoint(static_cast<uint16_t>(seq - 1));
        }
        break;
    case MAVLINK_MSG_ID_MISSION_REQUEST:
        {
            uint16_t seq = static_cast<uint16_t>(value.at("_40").asMap().at("seq").toUint64());
            qDebug() << "Waypoint request received" << seq << "state =" << _waypointsState;
            // Нулевая точка имеет специальное значение для Мавлинка
            if (seq == 0)
            {
                _mavlinkJob.fireAndSchedule([&]() { sendWaypointLoiter5s(0, *_guidedPoint); }); // Должна быть задана?
                if (_routeBuffer.isEmpty())
                {
                    toState(WaypointsState::WritingAck);
                }
            }
            else if (seq - 1 < _routeBuffer.size())
            {
                if (seq - 1 == _routeBuffer.size() - 1)
                {
                    _mavlinkJob.fireAndSchedule([&]() { sendWaypointLoiterUnlim(static_cast<uint16_t>(seq - 1), seq); });
                    toState(WaypointsState::WritingAck);
                }
                else
                {
                    // Т.к. нулевую точку мы уже передали, то номер сдвигаем на 1
                    _mavlinkJob.fireAndSchedule([&]() { sendWaypoint(static_cast<uint16_t>(seq - 1), seq); });
                }
            }
            else
            {
                qWarning() << "Waypoint request seq overflow: value =" << seq - 1 << "max =" << _routeBuffer.size();
            }
        }
        break;
    case MAVLINK_MSG_ID_MISSION_ACK:
        {
            uint8_t type = static_cast<uint8_t>(value.at("_47").asMap().at("type").toUint64());
            qDebug() << "Ack received:" << type << ackTypeToString(type);
            if (_waypointsState == WaypointsState::WritingAck)
            {
                if (type == 0)
                {
                    qDebug() << "Route writing success, rereading the route";
                    toState(WaypointsState::Outdated);
                    _mavlinkJob.deactivate();
                }
                else
                {
                    qDebug() << "Writing fails, rewrite";
                    //sendClearRoute();
                    _mavlinkJob.fireAndSchedule([&]() { sendWaypointCount(static_cast<uint16_t>(_routeBuffer.size() + 1)); });
                    toState(WaypointsState::Writing);
                }
            }
            else
            {
                qWarning() << "Waypoint ack has been received while in non-proper state";
            }
        }
        break;
    default:
        break;
    }
}

void RouteController::requestNextWaypoint(uint16_t seq)
{
    qDebug() << "Requesting waypoint" << seq;
    uint8_t buf[sizeof(uint16_t) + 2 * sizeof(uint8_t)];
    bmcl::MemWriter writer(buf);
    writer.writeUint16(seq);
    writer.writeUint8(_targetSystem);
    writer.writeUint8(_targetComponent);
    sendMavlinkPacket(_sender, MAVLINK_MSG_ID_MISSION_REQUEST, writer);
}

void RouteController::sendMissionRequestListPacket()
{
    qDebug() << "Sending mission request list";
    uint8_t buf[2 * sizeof(uint8_t)];
    bmcl::MemWriter args(buf);
    args.writeUint8(_targetSystem);
    args.writeUint8(_targetComponent);
    sendMavlinkPacket(_sender, MAVLINK_MSG_ID_MISSION_REQUEST_LIST, args);
}

void RouteController::provideTm()
{
    if (_waypointsState != WaypointsState::Ok)
        return;
    TmParams params;
    auto navRoutes = "Navigation.Routes";
    params.emplace_back(navRoutes, "count", 2);
    params.emplace_back(navRoutes, "activeRoute", _activeRoute);
    params.emplace_back(navRoutes, "nextPoint", _activeRoute == 0? 0 : _route.nextWaypoint());

    // "Маршрут" guided режима
    std::string guidedRouteTrait = "Navigation.Routes.Route" + std::to_string(0);
    addRouteToTmParams(params, guidedRouteTrait, 0, 1, false, _crc /* TODO это не лучший вариант, может быть хеш от lat-lon-alt? */, _guidedPoint.isSome() ? 1 : 0, 1, "Точка GUIDED режима");
    if (_guidedPoint.isSome())
        addPointToTmParams(params, guidedRouteTrait + ".Point" + std::to_string(0), *_guidedPoint);

    // Маршрут автопилота
    std::string routeTrait = "Navigation.Routes.Route" + std::to_string(1);
    addRouteToTmParams(params, routeTrait, 1, 1, false, _crc, _route.size(), 65535, "Основной маршрут");

    uint16_t pointIndex = 0;
    std::string traitNamePrefix = routeTrait + ".Point";
    for (auto& point : _route.points())
        addPointToTmParams(params, traitNamePrefix + std::to_string(pointIndex++), point);
    _messageSender->send<mcc::messages::TmParamList>(_deviceName, std::move(params));
}

bool RouteController::handleCommand(const Cmd & cmd)
{
    const std::string& trait = cmd.trait();
    const std::string& command = cmd.command();
    const uint32_t collationId = cmd.cmdId();
    if ("Navigation.Routes" == trait)
    {
        if ("createRoute" == command)
        {
            sendCommandStateEvent(_messageSender, cmd, CmdState::Failed, "Нельзя создать новый маршрут, команда не поддерживается");
            return true;
        }
        else if ("deleteRoute" == command)
        {
            // FIXME: имя маршрута в Мавлинке не имеет смысла?
            // uint16_t name = cmd.params().at(0).toUint();
            sendCommandStateEvent(_messageSender, cmd, CmdState::Failed, "Нельзя удалить активный маршрут, т.к. он последний");
            return true;
        }
        else if ("setActiveRoute" == command)
        {
            sendCommandStateEvent(_messageSender, cmd, CmdState::Failed, "Нельзя выбрать активный маршрут, т.к. он единственный");
            return true;
        }
        else if ("setActivePoint" == command)
        {
            uint16_t nextPoint = static_cast<uint16_t>(cmd.params().at(0).toUint());
            if (_route.size() <= nextPoint)
            {
                sendCommandStateEvent(_messageSender, cmd, CmdState::Failed, "Недопустимый индекс активной точки в маршруте");
                return true;
            }
            _route.setNextWaypoint(static_cast<std::size_t>(nextPoint == 0?  0 : nextPoint - 1));
            sendNextWaypoint(nextPoint); // TODO: Надо бы тоже в планировщик
        }
        else if ("beginRoute" == command)
        {
            uint16_t name = static_cast<uint16_t>(cmd.params().at(0).toUint());
            if (name != 0 && name != 1)
            {
                sendCommandStateEvent(_messageSender, cmd, CmdState::Failed,
                                      "Поддерживаются только маршруты с именем 0 и 1 для команды beginRoute по Мавлинку");
                return true;
            }
            toState(WaypointsState::WritingBuffer);
        }
        else if ("clearRoute" == command)
        {
            uint16_t name = static_cast<uint16_t>(cmd.params().at(0).toUint());
            switch (name)
            {
            case 0:
                qDebug() << "Clear GUIDED point";
                _guidedPoint.clear();
                break;
            case 1:
                qDebug() << "Clear AUTO route";
                _routeBuffer.clear();
                break;
            default:
                sendCommandStateEvent(_messageSender, cmd, CmdState::Failed,
                                      "Поддерживаются только маршруты с именем 0 и 1 для команды clearRoute по Мавлинку");
                return true;
            }
        }
        else if ("addRoutePoint" == command)
        {
            if (_waypointsState != WaypointsState::WritingBuffer)
            {
                sendCommandStateEvent(_messageSender, cmd, CmdState::Failed, "Команда addRoutePoint может быть исполнена только после исполнения beginRoute");
                return true;
            }
            uint16_t name = static_cast<uint16_t>(cmd.params().at(0).toUint());
            double latitude = cmd.params().at(1).toDouble();
            double longitude = cmd.params().at(2).toDouble();
            double altitude = cmd.params().at(3).toDouble();
            double speed = cmd.params().at(4).toDouble();
            uint32_t flags = static_cast<uint32_t>(cmd.params().at(5).toUint());
            RoutePoint point(latitude, longitude, altitude);
            switch (name)
            {
                case 0:
                    if (_guidedPoint.isSome())
                    {
                        sendCommandStateEvent(_messageSender, cmd, CmdState::Failed,
                                              "Превышение допустимого количества точек маршрута");
                        return true;
                    }
                    _guidedPoint = point;
                    break;
                case 1:
                    if (65535 <= _routeBuffer.size())
                    {
                        sendCommandStateEvent(_messageSender, cmd, CmdState::Failed,
                                              "Превышение допустимого количества точек маршрута");
                        return true;
                    }
                    _routeBuffer.push_back(std::move(point));
                    break;
                default:
                    sendCommandStateEvent(_messageSender, cmd, CmdState::Failed,
                                          "Поддерживаются только маршруты с именем 0 и 1 для команды addRoutePoint по Мавлинку");
                    return true;
            }
        }
        else if ("endRoute" == command)
        {
            if (_waypointsState != WaypointsState::WritingBuffer)
            {
                sendCommandStateEvent(_messageSender, cmd, CmdState::Failed,
                                      "Команда endRoute может быть исполнена только после исполнения beginRoute");
                return true;
            }
            uint16_t count = static_cast<uint16_t>(cmd.params().at(1).toUint());
            Q_UNUSED(count);
            //uint8_t isRing = cmd.params().at(2).toUint();
            // FIXME: Тупость, но сейчас на это завязан ГУЙ
            _crc++;
            // Включить чтение маршрута с автопилота
            _waypointsCount = 0;
            toState(WaypointsState::Writing);
            sendClearRoute();
            _mavlinkJob.fireAndSchedule([&]() { sendWaypointCount(static_cast<uint16_t>(_routeBuffer.size() + 1)); });
            qDebug() << "Starting to write route";
        }
        else
        {
            return false;
        }
        sendCommandStateEvent(_messageSender, cmd, CmdState::AcknowledgeReceived);
        return true;
    }
    return false;
}

void RouteController::sendNextWaypoint(uint16_t point)
{
    qDebug() << "Sending next waypoint";
    uint8_t buf[sizeof(uint16_t) + 2 * sizeof(uint8_t)];
    bmcl::MemWriter writer(buf);
    writer.writeUint16(point);
    writer.writeUint8(_targetSystem);
    writer.writeUint8(_targetComponent);
    sendMavlinkPacket(_sender, MAVLINK_MSG_ID_MISSION_SET_CURRENT, writer);
}

void RouteController::sendWaypointCount(uint16_t count)
{
    qDebug() << "Sending waypoint count" << count;
    uint8_t buf[sizeof(uint16_t) + 2 * sizeof(uint8_t)];
    bmcl::MemWriter writer(buf);
    writer.writeUint16(count);
    writer.writeUint8(_targetSystem);
    writer.writeUint8(_targetComponent);
    sendMavlinkPacket(_sender, MAVLINK_MSG_ID_MISSION_COUNT, writer);
}

void RouteController::sendWaypointLoiter5s(uint16_t waypointIndex, const RoutePoint &point)
{
    qDebug() << "Sending waypoint" << waypointIndex << "lat:" << QString::number(static_cast<float>(point.latitudeDeg()), 'f', 12)
        << "lon:" << QString::number(static_cast<float>(point.longitudeDeg()), 'f', 12)
        << "alt:" << QString::number(static_cast<float>(point.altitude()), 'f', 12);
    sendMissionItem(_sender, _targetSystem, _targetComponent, waypointIndex, MAV_FRAME_GLOBAL_TERRAIN_ALT, point,
                    waypointIndex == _routeBuffer.nextWaypoint(), waypointIndex == 0? 2 : 1, MAV_CMD_NAV_LOITER_TIME, 5.f);

}

void RouteController::sendWaypointLoiterUnlim(uint16_t waypointIndex, const RoutePoint &point)
{
    qDebug() << "Sending waypoint" << waypointIndex << "lat:" << QString::number(static_cast<float>(point.latitudeDeg()), 'f', 12)
    << "lon:" << QString::number(static_cast<float>(point.longitudeDeg()), 'f', 12)
    << "alt:" << QString::number(static_cast<float>(point.altitude()), 'f', 12);
    sendMissionItem(_sender, _targetSystem, _targetComponent, waypointIndex, MAV_FRAME_GLOBAL_TERRAIN_ALT, point,
                    waypointIndex == _routeBuffer.nextWaypoint(), waypointIndex == 0? 2 : 1, MAV_CMD_NAV_LOITER_UNLIM);

}

void RouteController::sendWaypoint(uint16_t waypointIndex)
{
    sendWaypoint(waypointIndex, waypointIndex);
}

void RouteController::sendWaypoint(uint16_t waypointIndex, uint16_t seq)
{
    sendWaypointLoiter5s(seq, _routeBuffer.at(waypointIndex));
}

void RouteController::sendWaypointLoiterUnlim(uint16_t waypointIndex, uint16_t seq)
{
    sendWaypointLoiterUnlim(seq, _routeBuffer.at(waypointIndex));
}

void RouteController::sendClearRoute()
{
    qDebug() << "Sending clear route";
    uint8_t buf[2 * sizeof(uint8_t)];
    bmcl::MemWriter writer(buf);
    writer.writeUint8(_targetSystem);
    writer.writeUint8(_targetComponent);
    sendMavlinkPacket(_sender, MAVLINK_MSG_ID_MISSION_CLEAR_ALL, writer);
}

void RouteController::toState(RouteController::WaypointsState newState)
{
    BMCL_ASSERT(_waypointsState != newState);
    qDebug() << stateToString(_waypointsState) << " -> " << stateToString(newState);
    _waypointsState = newState;
}

const char * RouteController::stateToString(WaypointsState state) {
    switch (state)
    {
    case Outdated:
        return "Outdated";
    case ReadingCount:
        return "ReadingCount";
    case Reading:
        return "Reading";
    case Ok:
        return "Ok";
    case WritingBuffer:
        return "WritingBuffer";
    case Writing:
        return "Writing";
    case WritingAck:
        return "WritingAck";
    }
}

void Route::resize(size_t newSize)
{
    BMCL_ASSERT(newSize <= _waypoints.size());
    std::size_t eraseCount = _waypoints.size() - newSize;
    _nextWaypoint = _nextWaypoint > 0 ? _nextWaypoint - eraseCount : 0;
    _waypoints.erase(_waypoints.begin(), _waypoints.begin() + eraseCount);
}

void RouteController::addPointToTmParams(messages::TmParams &params, const std::string & trait, const RoutePoint &point)
{
    params.emplace_back(trait, "latitude", point.latitudeDeg());
    params.emplace_back(trait, "longitude", point.longitudeDeg());
    params.emplace_back(trait, "altitude", point.altitude());
    params.emplace_back(trait, "speed", 0);
    params.emplace_back(trait, "flags", 0);
}

void RouteController::addRouteToTmParams(messages::TmParams &params, const std::string & trait, uint16_t name,
                                         uint8_t kind, bool isRing, uint16_t crc, size_t size, uint16_t maxCount,
                                         const char *info)
{
    params.emplace_back(trait, "name", name);
    params.emplace_back(trait, "kind", kind);
    params.emplace_back(trait, "isRing", isRing);
    params.emplace_back(trait, "crc16", crc);
    params.emplace_back(trait, "pointsCount", size);
    params.emplace_back(trait, "pointsMaxCount", maxCount);
    params.emplace_back(trait, "info", std::string(info));
}

bool Route::isEmpty() const
{
    return _waypoints.empty();
}

}
}
}