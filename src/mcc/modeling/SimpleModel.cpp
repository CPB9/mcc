/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include <chrono>
#include <thread>
#include "asio/steady_timer.hpp"
#include "asio/ip/udp.hpp"

#include "mcc/misc/TmParam.h"

#include "mcc/modeling/SimpleModel.h"

using mcc::misc::ParamValueList;
using mcc::misc::sendCommandStateEvent;

namespace mcc
{
namespace modeling
{

struct Asio
{
    explicit Asio(asio::io_service& service) :_socket(service), _timer(service){}
    ~Asio(){}
    asio::ip::udp::socket _socket;
    asio::steady_timer _timer;
};

void insertRoutePointToParamValueList(ParamValueList & paramValueList, const std::string& trait, const RoutePoint& point)
{
    paramValueList.insert(trait,
           {"latitude",          "longitude",          "altitude",       "speed"},
           {point.latitudeDeg(), point.longitudeDeg(), point.altitude(), point.speedMps()});
    paramValueList.insert(trait, "flags", point.flags());
}

SimpleModel::SimpleModel(const mcc::messages::LocalRouterPtr& router, const std::string &serviceName, const std::string &deviceName, uint16_t udpPort)
    : SimpleModel(router, serviceName, deviceName, udpPort, 43.01770, 42.69012, 200.0)
{
}

SimpleModel::~SimpleModel()
{
}

template<>
double SimpleModel::random(double from, double to)
{
    return std::uniform_real_distribution<>(from, to)(_modelingParameters.randomEngine());
}

template<>
int SimpleModel::random(int from, int to)
{
    return std::uniform_int_distribution<>(from, to)(_modelingParameters.randomEngine());
}

void SimpleModel::makeAllTmParameters(ParamValueList &paramValueList) const
{
    paramValueList._params.clear();
    makeTmParametersDevice(paramValueList);
    makeTmParametersNavigationDogPoint(paramValueList);
    makeTmParametersNavigationMode(paramValueList);
    makeTmParametersNavigationMotion(paramValueList);
    makeTmParametersNavigationRoutes(paramValueList);
}

void SimpleModel::makeTmParametersNavigationMotion(ParamValueList &paramValueList) const
{
    paramValueList.insert("Navigation.Motion", "throttle", static_cast<uint8_t>(std::abs(_throttle)));
    paramValueList.insert("Navigation.Motion",
                          {"latitude",   "longitude",   "altitude", "speed",   "pitch",   "heading",   "roll",   "accuracy"},
                          {_latitudeDeg, _longitudeDeg, _altitudeM, _speedMps, _pitchDeg, _headingDeg, _rollDeg, _accuracyM});
}

void SimpleModel::makeTmParametersNavigationMode(ParamValueList &paramValueList) const
{
    paramValueList.insert("Navigation.Mode", "activeMode", static_cast<uint8_t>(mode()));
}

void SimpleModel::makeTmParametersNavigationRoutes(ParamValueList &paramValueList) const
{
    paramValueList.insert("Navigation.Routes",
                          {"count",                               "activeRoute",        "nextPoint"},
                          {static_cast<uint16_t>(_routes.size()), activeRoute().name(), _nextPoint});

    std::size_t routeIndex = 0;
    for (auto& route: _routes)
    {
        std::string routeTrait = "Navigation.Routes.Route" + std::to_string(routeIndex);
        paramValueList.insert(routeTrait,
                              {"name", "kind", "isRing", "crc16", "pointsCount", "pointsMaxCount"},
                              {route.name(), static_cast<uint16_t>(route.kind()),
                               static_cast<uint16_t>(route.isRing()? 1 : 0), route.crc16(),
                               static_cast<uint16_t>(route.length()), route.maxLength()});
        paramValueList.insert(routeTrait, "info", route.info());
        uint16_t pointIndex = 0;
        std::string traitNamePrefix = routeTrait + ".Point";
        for (auto& point : route.points())
        {
            insertRoutePointToParamValueList(paramValueList, traitNamePrefix + std::to_string(pointIndex), point);
            pointIndex++;
        }
        routeIndex++;
    }
}

void SimpleModel::makeTmParametersNavigationDogPoint(ParamValueList &paramValueList) const
{
    RoutePoint dogPoint = {{_latitudeDeg, _longitudeDeg, _altitudeM}, _speedMps};
    if ((mode() == FlyingDeviceMode::RouteFlying || mode() == FlyingDeviceMode::Returning) && hasWaypoint())
    {
        dogPoint = routeWaypoint();
        double distToWaypointM(distanceToWaypointM(dogPoint));
        double k(_speedMps * 10. / distToWaypointM);
        dogPoint.setLatitudeDeg(_latitudeDeg + (dogPoint.latitudeDeg() - _latitudeDeg) * k);
        dogPoint.setLongitudeDeg(_longitudeDeg + (dogPoint.longitudeDeg() - _longitudeDeg) * k);
    }
    insertRoutePointToParamValueList(paramValueList, "Navigation.DogPoint", dogPoint);
}

void SimpleModel::makeTmParametersDevice(ParamValueList &paramValueList) const
{
    paramValueList.insert("Device",
                          {"signalLevel",                          "batteryLevel"},
                          {static_cast<uint8_t>(_signalLevelPer), static_cast<uint8_t>(_batteryLevelPer)});
}

void SimpleModel::advanceWaypoint(double prevLatitudeDeg, double prevLongitudeDeg)
{
    bool isReturning = (FlyingDeviceMode::Returning == mode());
    while (true)
    {
        const RoutePoint & currentWaypoint(routeWaypoint());
        double waypointLatDeg(currentWaypoint.latitudeDeg()), waypointLongDeg(currentWaypoint.longitudeDeg());
        double minDistanceToWaypointM(
            qMin(qMin(degToM(distanceToLineSegment(prevLatitudeDeg, prevLongitudeDeg, _latitudeDeg, _longitudeDeg, waypointLatDeg, waypointLongDeg)),
                      degToM(distance(prevLatitudeDeg, prevLongitudeDeg, waypointLatDeg, waypointLongDeg))),
                 degToM(distance(_latitudeDeg, _longitudeDeg, waypointLatDeg, waypointLongDeg))));

        if (minDistanceToWaypointM > _waypointCheckDistanceM)
        {
            break;
        }
        if (_routes.size() <= _activeRoute)
        {
            // Нет маршрута или неправильный активный маршрут
            break;
        }
        Route& activeRoute(_routes.at(_activeRoute));
        if (isReturning)
        {
            if (_nextPoint > 0)
            {
                _nextPoint -= 1;
            }
            else if (activeRoute.isRing())
            {
                _nextPoint = static_cast<uint16_t>(activeRoute.length() - 1);
            }
            else
            {
                break;
            }
        }
        else
        {
            if (_nextPoint < activeRoute.length() - 1)
            {
                _nextPoint += 1;
            }
            else if (activeRoute.isRing())
            {
                _nextPoint = 0;
            }
            else
            {
                break;
            }
        }
    }
}

void SimpleModel::step()
{
    FlyingDeviceMode m = mode();
    if (FlyingDeviceMode::TakingOff == m && _altitudeM > _deviceModelingParameters.takingOffAltitudeM())
    {
        bool result(_modesStateMachine->activate(Command("Navigation.freeFlying")));
        Q_ASSERT(result);
        Q_UNUSED(result);
    }
    if (FlyingDeviceMode::Landing == m && _altitudeM < .1)
    {
        bool result(_modesStateMachine->activate(Command("Navigation.endLanding")));
        Q_ASSERT(result);
        Q_UNUSED(result);
    }
    updateAccuracy();
    updateConnectionAvailable();
    updateHeading();
    updateAcceleration();
    updateSpeed();
    advanceCoords();
    updateRoll();
    updatePitch();
    updateAltitude();
    updateBatteryLevel();
    updateSignalLevel();

    _tick++;

    _asio->_timer.expires_from_now(std::chrono::milliseconds(_modelingParameters.sleepMs()));
    _asio->_timer.async_wait([&](const asio::error_code& error) { step(); });
}

void SimpleModel::advanceCoords()
{
    advanceLatitudeLongitudeDeg(_latitudeDeg, _longitudeDeg, _headingDeg, _speedMps * _tickS);
    if (hasWaypoint())
    {
        double prevLatitudeDeg(_latitudeDeg), prevLongitudeDeg(_longitudeDeg);
        advanceWaypoint(prevLatitudeDeg, prevLongitudeDeg);
    }
}

void SimpleModel::updateAccuracy()
{
    double limit(qMin(0.99, _tickS));
    if (random(limit))
    {
        _accuracyM = std::abs(std::sin(misc::degreesToRadians(_tick + random(0., limit * 360.)))) * 1000.;
    }
}

void SimpleModel::updateConnectionAvailable()
{
    // Временно отключено, чтобы не мешало тестированию
    return;
    /*
    if (random(qMin(.99, _deviceModelingParameters.badSignal()? 1. : (_connectionAvailable ? .1 : .5)) * _tickS * .02))
    {
        _connectionAvailable = !_connectionAvailable;
    }
    */
}

Option<double> SimpleModel::targetHeadingDeg() const
{
    if (mode() == FlyingDeviceMode::GuidedFlying && _guidedFlyingTargetOrientation.isSome())
    {
        return _guidedFlyingTargetOrientation->headingDeg();
    }
    else if (hasWaypoint())
    {
        return normalizeHeadingDeg(angleToWaypointDeg(waypointCoordinates()));
    }
    return None;
}

void SimpleModel::updateHeading()
{
    Option<double> _targetHeadingDeg(targetHeadingDeg());
    if (FlyingDeviceMode::Waiting == mode())
    {
        _headingDeg += _deviceModelingParameters.rotateSpeedDegps() * _tickS;
    }
    else if (_targetHeadingDeg.isSome())
    {
        double waypointAngleDeg = _targetHeadingDeg.unwrap();
        double headingWaypointAnglesDeltaDeg = anglesDeltaDeg(_headingDeg, waypointAngleDeg);
        _headingDeg += (headingWaypointAnglesDeltaDeg < 0. ? -1. : 1.)
                       * std::min(std::abs(headingWaypointAnglesDeltaDeg),
                              _deviceModelingParameters.rotateSpeedDegps() * (1. - _speedMps / maxSpeedMps() * .3))
                       * _tickS;
    }
    _headingDeg = normalizeHeadingDeg(_headingDeg);
}

double SimpleModel::computeTargetRollDegForTargetHeadingDeg(double targetHeadingDeg) const
{
    double headingWaypointAnglesDeltaDeg(anglesDeltaDeg(_headingDeg, targetHeadingDeg));
    double rollRad = std::max(std::min(misc::degreesToRadians(headingWaypointAnglesDeltaDeg), M_PI_4), -M_PI_4);
    return normalizeRollDeg(misc::radiansToDegrees(std::tan(rollRad)));
}

Option<double > SimpleModel::targetRollDeg() const
{
    if (mode() == FlyingDeviceMode::GuidedFlying && _guidedFlyingTargetOrientation.isSome())
    {
        return _guidedFlyingTargetOrientation->headingDeg();
    }
    else if (hasWaypoint())
    {
        Option<double> _targetHeadingDeg(targetHeadingDeg());
        BMCL_ASSERT_MSG(_targetHeadingDeg.isSome(), "target heading must be present");
        return computeTargetRollDegForTargetHeadingDeg(_targetHeadingDeg.unwrap());
    }
    return None;
}

void SimpleModel::updateRoll()
{
    Option<double> _targetRollDeg(targetRollDeg());
    if (FlyingDeviceMode::Waiting == mode())
    {
        _rollDeg = M_PI_4;
    }
    else if (_targetRollDeg.isSome())
    {
        _rollDeg += (_targetRollDeg.unwrap() - _rollDeg) * random(0.1, 0.9);
    }
}

void SimpleModel::updatePitch()
{
    double tickDeg(static_cast<double>(_tick));
    double tickRad(misc::degreesToRadians(tickDeg));
    _pitchDeg = (std::cos(tickRad / 4) * .5 + std::sin(tickRad / 4) * .2) * 20.;
}

void SimpleModel::updateAltitude()
{
    if (isBatteryLow() || FlyingDeviceMode::EngineReady == mode())
    {
        _altitudeM -= 10. * _tickS;
    }
    else
    {
        switch (mode())
        {
            case FlyingDeviceMode::TakingOff:
                _altitudeM += 10. * _tickS;
                break;
            case FlyingDeviceMode::Landing:
                _altitudeM -= 20. * _tickS;
                break;
            case FlyingDeviceMode::GuidedFlying:
            case FlyingDeviceMode::RouteFlying: {
                if (hasWaypoint())
                {
                    double waypointAltitudeM = waypointCoordinates().altitude();
                    if (waypointAltitudeM == 0.)
                    {
                        waypointAltitudeM = .00001;
                    }
                    double altitudeDeltaPer = (waypointAltitudeM - _altitudeM) / waypointAltitudeM;
                    _altitudeM += altitudeDeltaPer * _speedMps * .05 * _tickS;
                }
                break;
            }
            default:
                break;
        }
    }
    _altitudeM = qMax(_altitudeM, 0.);
}

void SimpleModel::updateBatteryLevel()
{
    _batteryLevelPer -= _throttle / 5000. * _tickS;
}

void SimpleModel::updateSignalLevel()
{
    static double shift(random(0., 360.));
    double tickDeg(_tick + shift + random(0., 9.));
    double tickRad(misc::degreesToRadians(tickDeg));
    _signalLevelPer = std::abs(std::sin(tickRad) * std::cos(2 * tickRad) + .4 * std::cos(tickRad * 64)) / 1.4 * 100.;
}

void SimpleModel::updateAcceleration()
{
    double maxAccelerationMpsps(_deviceModelingParameters.maxAccelerationMpsps());
    if (isBatteryLow())
    {
    }
    switch(mode())
    {
        case FlyingDeviceMode::Waiting:
            _accelerationMpsps = 0;
            _throttle = 30.;
            break;
        case FlyingDeviceMode::TakingOff:
            _accelerationMpsps = maxAccelerationMpsps;
            _throttle = 100.;
            break;
        case FlyingDeviceMode::Landing:
            _accelerationMpsps = _speedMps > 0. ? -maxAccelerationMpsps : 0.;
            _throttle = 100.;
            break;
        case FlyingDeviceMode::GuidedFlying:
        case FlyingDeviceMode::RouteFlying:
            _accelerationMpsps = maxAccelerationMpsps;
            _throttle = 100.;
            if (hasWaypoint())
            {
                double waypointSpeedMps = FlyingDeviceMode::GuidedFlying == mode() ? maxSpeedMps() : routeWaypoint().speedMps();
                if (waypointSpeedMps == 0.)
                {
                    waypointSpeedMps = .000001;
                }
                double speedPer(_speedMps / waypointSpeedMps);
                _accelerationMpsps += ((speedPer > .9) ? (speedPer > 1.1 ? - maxAccelerationMpsps : .01 * maxAccelerationMpsps) : maxAccelerationMpsps) * _tickS;
                _accelerationMpsps = qMin(qMax(_accelerationMpsps, - maxAccelerationMpsps), maxAccelerationMpsps);
                _throttle = _accelerationMpsps < 0.? 0. : qMin(_speedMps / maxSpeedMps() * 70. + qAbs(_accelerationMpsps) / maxAccelerationMpsps * 50., 100.);
            }
            break;
        default:
            _accelerationMpsps = -maxAccelerationMpsps;
            _throttle = 0;
            break;
    }
}

void SimpleModel::updateSpeed()
{
    _speedMps += _accelerationMpsps * _tickS;
    _speedMps = qMax(qMin(_speedMps, maxSpeedMps()), 0.);
}

bool SimpleModel::hasWaypoint() const
{
    if (mode() == FlyingDeviceMode::GuidedFlying)
    {
        return _guidedFlyingTargetCoordinates.isSome();
    }
    else
    {
        return (FlyingDeviceMode::RouteFlying == mode() || FlyingDeviceMode::Returning == mode()) && activeRoute().length() > _nextPoint;
    }
}

void SimpleModel::process(std::unique_ptr<mcc::messages::Cmd>&& cmdX)
{
    const std::string& trait = cmdX->trait();
    const std::string& command = cmdX->command();
    const uint32_t collationId(cmdX->cmdId());
    Command cmd(trait + "." + command);

//     sendCommandStateEvent(trait, command, collationId, CmdState::RoutedForDelivery);
//     sendCommandStateEvent(trait, command, collationId, CmdState::WaitingInDeliveryQueue);
//     sendCommandStateEvent(trait, command, collationId, CmdState::SentToDevice);
    if ("Navigation.Routes" == trait)
    {
        if ("createRoute" == command)
        {
            uint16_t name = cmdX->params().at(0).toUint();
            _routeBuffer.setName(name);
        }
        else if ("deleteRoute" == command)
        {
            uint16_t name = cmdX->params().at(0).toUint();
            auto it(findRouteByName(name));
            if (it == _routes.end())
            {
                sendRouteNotFound(trait, *cmdX, name);
                return;
            }
            if (_activeRoute == it - _routes.begin())
            {
                sendCommandStateEvent(_out, *cmdX, CmdState::Failed, "Нельзя удалить активный маршрут");
                return;
            }
            _routes.erase(it);
        }
        else if ("setActiveRoute" == command)
        {
            uint16_t name = cmdX->params().at(0).toUint();
            auto it(findRouteByName(name));
            if (it == _routes.end())
            {
                sendRouteNotFound(trait, *cmdX, name);
                return;
            }
            _activeRoute = static_cast<uint16_t>(it - _routes.begin());
            _nextPoint = 0;
        }
        else if ("setActivePoint" == command)
        {
            uint16_t nextPoint = cmdX->params().at(0).toUint();
            if (activeRoute().length() <= nextPoint)
            {
                sendCommandStateEvent(_out, *cmdX, CmdState::Failed, "Недопустимый индекс активной точки в маршруте");
                return;
            }
            _nextPoint = nextPoint;
        }
        else if ("beginRoute" == command)
        {
            uint16_t name = cmdX->params().at(0).toUint();
            _routeBuffer.setName(name);
        }
        else if ("clearRoute" == command)
        {
            // TODO: Тут ещё параметр в команде, но видимо это ошибка, т.к. чистить надо всегда буфер
            _routeBuffer.clear();
        }
        else if ("addRoutePoint" == command)
        {
            //uint16_t name = cmdX->params().at(0).toUint();
            double latitude = cmdX->params().at(1).toDouble();
            double longitude = cmdX->params().at(2).toDouble();
            double altitude = cmdX->params().at(3).toDouble();
            double speed = cmdX->params().at(4).toDouble();
            uint32_t flags = cmdX->params().at(5).toUint();
            if (_routeBuffer.maxLength() <= _routeBuffer.length())
            {
                sendCommandStateEvent(_out, *cmdX, CmdState::Failed, "Превышение допустимого количества точек маршрута");
                return;
            }
            _routeBuffer.appendPoint(latitude, longitude, altitude, speed, flags);
        }
        else if ("endRoute" == command)
        {
            uint16_t name = cmdX->params().at(0).toUint();
            uint16_t count = cmdX->params().at(1).toUint();
            Q_UNUSED(count);
            uint8_t  isRing = cmdX->params().at(2).toUint();
            _routeBuffer.setIsRing(isRing != 0);
            _routeBuffer.setName(name);
            auto it(findRouteByName(name));
            _routeBuffer.setCrc16(static_cast<uint16_t>(qrand()));
            if (it != _routes.end())
            {
                bool isActive(_routes.at(_activeRoute).name() == name);
                if (isActive && _routeBuffer.length() < 1)
                {
                    sendCommandStateEvent(_out, *cmdX, CmdState::Failed, "Нельзя удалить все точки из активного маршрута");
                    return;
                }
                *it = _routeBuffer;
                if (isActive)
                {
                    _nextPoint = 0;
                }
            }
            else
            {
                _routes.push_back(_routeBuffer);
            }
        }
        sendCommandStateEvent(_out, *cmdX, CmdState::AcknowledgeReceived);
    }
    else if ("Navigation.Motion" == trait)
    {
        if (_modesStateMachine->mode() != FlyingDeviceMode::GuidedFlying)
        {
            sendCommandStateEvent(_out, *cmdX, CmdState::Failed, "Недопустимая команда для текущего режима");
            return;
        }
        if ("moveTo" == command)
        {
            double latitudeDeg = cmdX->params().at(1).toDouble();
            double longitudeDeg = cmdX->params().at(2).toDouble();
            double altitudeM = cmdX->params().at(3).toDouble();
            _guidedFlyingTargetCoordinates.emplace(latitudeDeg, longitudeDeg, altitudeM);
            _guidedFlyingTargetOrientation.clear();
        }
        else if ("rotateTo" == trait)
        {
            double headingDeg = cmdX->params().at(1).toDouble();
            double pitchDeg = cmdX->params().at(2).toDouble();
            double rollDeg = cmdX->params().at(3).toDouble();
            _guidedFlyingTargetOrientation.emplace(headingDeg, pitchDeg, rollDeg);
            _guidedFlyingTargetCoordinates.clear();
        }
        else
        {
            sendCommandStateEvent(_out, *cmdX, CmdState::Failed, "Неизвестная команда");
            return;
        }
        sendCommandStateEvent(_out, *cmdX, CmdState::AcknowledgeReceived);
        return;
    }
    else if ("Tm" == trait)
    {
    }
    else
    {
        if (_modesStateMachine->isValidAction(cmd))
        {
            static std::bernoulli_distribution failDist(.1);
            if (failDist(_modelingParameters.randomEngine()))
            {
                sendCommandStateEvent(_out, *cmdX, CmdState::Failed, "Сбой исполнения команды");
                return;
            }
            if (_modesStateMachine->activate(cmd))
            {
                sendCommandStateEvent(_out, *cmdX, CmdState::AcknowledgeReceived);
                return;
            }
            else
            {
                sendCommandStateEvent(_out, *cmdX, CmdState::Failed, "Недопустимая команда для текущего режима");
                return;
            }
        }
        else
        {
            sendCommandStateEvent(_out, *cmdX, CmdState::Failed, "Неизвестная команда");
            return;
        }
    }
}

bool SimpleModel::pre()
{
    subscribeForCommands();
    asio::error_code ec;

    _ioService.reset(new asio::io_service);
    _asio.reset(new Asio(*_ioService));
    _asio->_socket.open(asio::ip::udp::v4(), ec);
    _asio->_socket.bind(asio::ip::udp::endpoint(asio::ip::udp::v4(), _udpPort), ec);
    _asio->_socket.async_wait(asio::ip::udp::socket::wait_read, std::bind(&SimpleModel::command, this));

    step();
    return true;
}

void SimpleModel::post()
{
    unsubscribeFromCommands();
    _asio->_socket.cancel();
    _asio->_timer.cancel();
    _asio.reset();
    _ioService.reset();
}

void SimpleModel::tick()
{
    while (isRunning_() && _ioService->run_one())
    {
    }
}

SimpleModel::SimpleModel(const mcc::messages::LocalRouterPtr& router, const std::string &serviceName, const std::string &deviceName,
                         uint16_t udpPort, double latitude, double longitude, double altitude, Route const &route,
                         ModelingParameters modelingParameters, DeviceModelingParameters deviceModelingParameters)
        : Model(router, serviceName, deviceName, modelingParameters.tickS())
        , PointCoordinates(latitude, longitude, altitude)
        , Orientation(0., 0., 0.)
        , _udpPort(udpPort)
        , _routes({route})
        , _speedMps(0.)
        , _accelerationMpsps(0.)
        , _prevDistanceM(std::numeric_limits<double>::max())
        , _throttle(0.)
        , _signalLevelPer(0.)
        , _waypointCheckDistanceM(50.)
        , _tick(0)
        , _activeRoute(0)
        , _nextPoint(0)
        , _modelingParameters(modelingParameters)
        , _deviceModelingParameters(deviceModelingParameters)
        , _batteryLevelPer(std::uniform_real_distribution<>(50., 100.)(modelingParameters.randomEngine()))
        , _connectionAvailable(true)
        , _accuracyM(30.)
{
    std::uniform_int_distribution<uint16_t> dist;
    for (auto& aRoute : _routes)
    {
        aRoute.setCrc16(dist(_modelingParameters.randomEngine()));
    }
    using M = FlyingDeviceMode;
    using T = FlyingDeviceModesFsmTransition;
    using Nav = NavigationCommands;
    _modesStateMachine.reset(new FlyingDeviceModesFsm(M::RouteFlying, {
            T(M::Off, Nav::start(), M::PayloadReady),
            T(M::PayloadReady, Nav::startEngine(), M::EngineReady),
            T(M::EngineReady, Nav::takeOff(), M::TakingOff),
            T(M::TakingOff, Nav::guidedFlying(), M::GuidedFlying, nullptr,
                                        [&]() { return _altitudeM > _deviceModelingParameters.takingOffAltitudeM(); }),
            T(M::GuidedFlying, Nav::flyRoute(), M::RouteFlying),
            T(M::RouteFlying, Nav::guidedFlying(), M::GuidedFlying),
            T(M::RouteFlying, Nav::wait(), M::Waiting),
            T(M::Waiting, Nav::_return(), M::Returning,
                                        [&]() {
                                            BMCL_ASSERT_MSG(_routes.size() > _activeRoute, "invalid active route");
                                            if (_nextPoint > 0)
                                            {
                                                _nextPoint -= 1;
                                            }
                                        }),
            T(M::Returning, Nav::land(), M::Landing),
            T(M::Landing, Nav::endLanging(), M::EngineReady, nullptr,
                                        [&]() { return _altitudeM < .1; }),
            T(M::Landing, Nav::cancelLanging(), M::TakingOff),
            T(M::EngineReady, Nav::stopEngine(), M::PayloadReady),
            T(M::PayloadReady, Nav::stop(), M::Off)
    }));
}

void SimpleModel::command()
{
    asio::ip::udp::endpoint endpoint;

    while (_asio->_socket.available() > 0)
    {
        std::size_t available = _asio->_socket.available();
        _channelBuffer.resize(available);
        asio::error_code ec;
        std::size_t received = _asio->_socket.receive_from(asio::buffer(_channelBuffer, available), endpoint, 0, ec);
        if (ec)
            qDebug() << ec.message().c_str();
        if (received == 0)
            return;
        _channelBuffer.resize(received);
        if (!_connectionAvailable)
        {
            // Пропадание связи, не шлем телеметрию и не обрабатываем команду (или ввести случаную обработку без тм?)
            return;
        }
    }

    auto p = mcc::messages::Message::deserialize(_channelBuffer.data(), _channelBuffer.size());
    if (p)
    {
        chooseProcessor(std::move(p));

        makeAllTmParameters(_paramList);
        mcc::messages::TmParamList list(_deviceName, _paramList._params);

        asio::error_code ec;
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        _asio->_socket.send_to(asio::buffer(list.serialize()), endpoint, 0, ec);
    }
    _asio->_socket.async_wait(asio::ip::udp::socket::wait_read, std::bind(&SimpleModel::command, this));
}

const PointCoordinates &SimpleModel::waypointCoordinates() const
{
    if (mode() == FlyingDeviceMode::GuidedFlying)
    {
        BMCL_ASSERT_MSG(_guidedFlyingTargetCoordinates.isSome(), "free flying coordinates not set");
        return _guidedFlyingTargetCoordinates.unwrap();
    }
    return routeWaypoint();
}

}
}
