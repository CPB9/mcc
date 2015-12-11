/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#pragma once

#include <functional>

#include "mcc/core/decode/Registry.h"

#include "mcc/misc/Channel.h"
#include "mcc/misc/Route.h"
#include "mcc/messages/MessageSender.h"
#include "mcc/messages/Tm.h"

#include "mcc/encoder/mavlink/mavlink_utils.h"

namespace mcc
{
namespace encoder
{
namespace mavlink
{

class RoutePoint : public mcc::misc::PointCoordinates
{
public:
    RoutePoint(double latitudeDeg, double longitudeDeg, double altitudeM)
            : mcc::misc::PointCoordinates(latitudeDeg, longitudeDeg, altitudeM)
    {}
};

class Route
{
public:
    Route(std::vector<RoutePoint> && waypoints = std::vector<RoutePoint>())
            : _nextWaypoint(0)
            , _waypoints(std::move(waypoints))
    {}
    void clear() { _waypoints.clear(); _nextWaypoint = 0; }
    void reserve(std::size_t size) { _waypoints.reserve(size); }
    std::size_t size() const { return _waypoints.size(); }
    void push_back(RoutePoint && point) { _waypoints.push_back(point); }
    std::size_t nextWaypoint() const { return _nextWaypoint; }
    void setNextWaypoint(std::size_t nextWaypoint) { BMCL_ASSERT(nextWaypoint == 0 || nextWaypoint < size()); _nextWaypoint = nextWaypoint; }
    const std::vector<RoutePoint> & points() const { return _waypoints; }
    const RoutePoint & at(std::size_t index) const { return _waypoints.at(index); }
    void resize(size_t newSize);
    bool isEmpty() const;

private:
    std::size_t _nextWaypoint;
    std::vector<RoutePoint> _waypoints;
};

template<typename D = std::chrono::milliseconds, typename C = std::chrono::steady_clock, typename T = typename C::time_point>
class Timer
{
public:
    explicit Timer(const D duration, bool isActive = true, const T start = C::now())
        : _isActive(isActive), _start(start), _duration(duration)
    {}
    bool isActive() const { return _isActive; }
    D duration() const { return _duration; }
    bool isTimeInThePast() const { return C::now() > _start + _duration; }
    void reschedule(const T start = C::now()) { _start = start; _isActive = true; }
    void deactivate() { _isActive = false; }

private:
    bool _isActive;
    T _start;
    D _duration;
};

template<typename D = std::chrono::milliseconds, typename C = std::chrono::steady_clock, typename T = typename C::time_point>
class Job
{
public:
    explicit Job(const Timer<D, C, T> timer, const std::function<void()> action = nullptr)
            : _timer(timer), _action(action)
    {}
    bool isActiveAndTimeInThePast() const { return _timer.isActive() && _timer.isTimeInThePast(); }
    void fireAndSchedule(const std::function<void()> action) { _action = action; fireAndReschedule(); }
    void fireAndReschedule() { if (_action != nullptr) _action(); _timer.reschedule(); }
    void deactivate() { _timer.deactivate(); }
    void handle() { if (isActiveAndTimeInThePast()) fireAndReschedule(); }

private:
    Timer<D, C, T> _timer;
    std::function<void()> _action;
};

class RouteController
{
    typedef enum { Outdated, ReadingCount, Reading, Ok, WritingBuffer, Writing, WritingAck } WaypointsState;

    using Sender = mcc::misc::Sender<std::vector<uint8_t>>;

public:
    RouteController(Sender & sender, mcc::messages::MessageSender & messageSender, std::string & deviceName);
    void tick();
    void handleTmMessage(uint8_t messageId, decode::MapVariant & value);
    const Route & route() const { return _route; }

    bool handleCommand(const messages::Cmd &cmd);

private:
    Sender _sender;
    mcc::messages::MessageSender _messageSender;
    std::string _deviceName;
    std::shared_ptr<decode::Component> _pixhawkComponent;
    uint8_t _targetSystem, _targetComponent;
    WaypointsState _waypointsState;
    uint_fast16_t _waypointsCount;
    Route _route, _routeBuffer;
    uint_fast8_t _activeRoute;
    uint16_t _crc;
    bmcl::Option<RoutePoint> _guidedPoint;
    Job<> _mavlinkJob;

    void sendMissionRequestListPacket();
    void requestNextWaypoint(uint16_t seq);
    void provideTm();

    void sendNextWaypoint(uint16_t point);
    void sendWaypointCount(uint16_t count);

    void sendWaypoint(uint16_t waypointIndex, uint16_t seq);
    void sendWaypointLoiterUnlim(uint16_t waypointIndex, uint16_t seq);
    void sendWaypointLoiter5s(uint16_t waypointIndex, const RoutePoint &point);
    void sendWaypointLoiterUnlim(uint16_t waypointIndex, const RoutePoint &point);
    void sendWaypoint(uint16_t waypointIndex);

    void sendClearRoute();

    void toState(WaypointsState newState);

    const char * stateToString(WaypointsState state);

    void addPointToTmParams(messages::TmParams &params, const std::string &trait, const RoutePoint &point);

    void addRouteToTmParams(messages::TmParams &params, const std::string & trait, uint16_t name, uint8_t kind, bool isRing, uint16_t crc, size_t size, uint16_t maxCount,
                            const char *info);
};

}
}
}
