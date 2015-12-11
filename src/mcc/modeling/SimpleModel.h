/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#pragma once

#include <math.h>
#include <string>
#include <iostream>
#include <limits>
#include <map>
#include <set>
#include <functional>
#include <utility>
#include <memory>
#include <unordered_map>
#include <random>
#include <string>

#include "bmcl/Assert.h"

#include "mcc/Names.h"
#include "mcc/misc/TmParam.h"
#include "mcc/misc/Route.h"
#include "mcc/misc/tm_utils.h"
#include "mcc/messages/Tm.h"
#include "mcc/messages/MessageSender.h"
#include "mcc/messages/Cmd.h"
#include "mcc/modeling/Model.h"
#include "mcc/modeling/flying_device.h"

namespace asio { class io_service; }

using namespace mcc;
using namespace mcc::messages;

using bmcl::Option;
using bmcl::None;

using mcc::misc::ParamValueList;
using mcc::misc::Orientation;

namespace mcc
{
namespace modeling
{

struct Asio;

void insertRoutePointToParamValueList(ParamValueList & paramValueList, const std::string& trait, const RoutePoint& point);

class ModelingParameters
{
public:
    explicit ModelingParameters(double tickS = .25, uint32_t sleepMs = 250, std::initializer_list<uint_fast64_t> seedPrams = {0, 30, 151, 2})
            : _tickS(tickS), _sleepMs(sleepMs)
            {
                std::seed_seq seed(seedPrams);
                _randomEngine.seed(seed);
            }
    double tickS() const { return _tickS; }
    uint32_t sleepMs() const { return _sleepMs; }
    void setSleepMs(uint32_t sleepMs) { _sleepMs = sleepMs; }
    std::default_random_engine &randomEngine() { return _randomEngine; }
private:
    double _tickS;
    uint32_t _sleepMs;
    std::default_random_engine _randomEngine;
};

class DeviceModelingParameters
{
public:
    explicit DeviceModelingParameters(double maxSpeedMps = 60., double maxAccelerationMpsps = 20,
                                      double takingOffAltitudeM = 50., double rotateSpeedDegps = 30.,
                                      bool badSignal = false)
    : _maxSpeedMps(maxSpeedMps), _maxAccelerationMpsps(maxAccelerationMpsps), _takingOffAltitudeM(takingOffAltitudeM),
      _rotateSpeedDegps(rotateSpeedDegps), _badSignal(badSignal) {}
    double maxAccelerationMpsps() const { return _maxAccelerationMpsps; }
    double maxSpeedMps() const { return _maxSpeedMps; }
    double takingOffAltitudeM() const { return _takingOffAltitudeM; }
    double rotateSpeedDegps() const { return _rotateSpeedDegps; }
    bool badSignal() const { return _badSignal; }
private:
    double _maxSpeedMps, _maxAccelerationMpsps, _takingOffAltitudeM, _rotateSpeedDegps;
    bool _badSignal;
};

class SimpleModel : public Model, public PointCoordinates, public Orientation
{
public:
    SimpleModel(const mcc::messages::LocalRouterPtr& router, const std::string & serviceName, const std::string & deviceName,
                uint16_t udpPort, double latitude, double longitude, double altitude,
    const Route & route = Route(1, SimplifiedRouteKind::Auto, true, 0, 50, "Default route",
        {
            {{43.10098, 42.87415, 500.}, 600., 1},
            {{43.42899, 42.11609, 500.}, 600.},
            {{43.60824, 42.93457, 500.}, 600., 8},
            {{43.92955, 42.22046, 500.}, 600.},
            {{44.15068, 42.88513, 500.}, 600.},
            {{41.91863, 43.76953, 500.}, 600.},
        }),
    ModelingParameters modelingParameters = ModelingParameters(), DeviceModelingParameters deviceModelingParameters = DeviceModelingParameters());

    SimpleModel(const mcc::messages::LocalRouterPtr& router, const std::string &serviceName, const std::string &deviceName, uint16_t udpPort);

    virtual ~SimpleModel();

    void setSleepMs(uint32_t sleepMs) { _modelingParameters.setSleepMs(sleepMs); }

    bool pre() override;
    void tick() override;
    void post() override;

    void step() override;

    void process(std::unique_ptr<mcc::messages::Cmd>&& cmdX) override;

    void appendRoute(const Route &&route)
    {
        BMCL_ASSERT_MSG(findRouteByName(route.name()) == _routes.end(), std::string("duplicate route name " + std::to_string(route.name())).c_str());
        _routes.emplace_back(std::move(route));
    }

    void command();

private:

    std::unique_ptr<asio::io_service> _ioService;
    std::unique_ptr<Asio> _asio;
    std::vector<uint8_t> _channelBuffer;
    uint16_t _udpPort;

    std::vector<Route> _routes;
    Route _routeBuffer;
    double _speedMps, _accelerationMpsps, _prevDistanceM, _throttle, _signalLevelPer, _waypointCheckDistanceM;
    uint64_t _tick;
    uint16_t _activeRoute, _nextPoint;
    ModelingParameters _modelingParameters;
    DeviceModelingParameters _deviceModelingParameters;
    std::unique_ptr<FlyingDeviceModesFsm> _modesStateMachine;
    double _batteryLevelPer;
    bool _connectionAvailable;
    double _accuracyM;
    ParamValueList _paramList;
    Option<PointCoordinates> _guidedFlyingTargetCoordinates;
    Option<Orientation> _guidedFlyingTargetOrientation;

    void makeAllTmParameters(ParamValueList &params) const;
    void makeTmParametersDevice(ParamValueList &params) const;
    void makeTmParametersNavigationRoutes(ParamValueList &params) const;
    void makeTmParametersNavigationMode(ParamValueList &params) const;
    void makeTmParametersNavigationDogPoint(ParamValueList &params) const;
    void makeTmParametersNavigationMotion(ParamValueList &params) const;

    Option<double> targetHeadingDeg() const;
    Option<double> targetRollDeg() const;
    double computeTargetRollDegForTargetHeadingDeg(double targetHeadingDeg) const;

    void advanceWaypoint(double prevLatitudeDeg, double prevLongitudeDeg);
    void advanceCoords();
    void updateAccuracy();
    void updateConnectionAvailable();
    void updateHeading();
    void updateRoll();
    void updatePitch();
    void updateAltitude();
    void updateBatteryLevel();
    void updateSignalLevel();
    void updateAcceleration();
    void updateSpeed();
    bool hasWaypoint() const;

    const Route & activeRoute() const { BMCL_ASSERT_MSG(_routes.size() > _activeRoute, "invalid active route"); return _routes.at(_activeRoute); }
    const RoutePoint & routeWaypoint() const { return activeRoute().point(_nextPoint); }
    const PointCoordinates & waypointCoordinates() const;
    double maxSpeedMps() const { return _deviceModelingParameters.maxSpeedMps(); }
    bool isBatteryLow() const { return _batteryLevelPer < .01; }

    FlyingDeviceMode mode() const { return _modesStateMachine->mode(); }

    template<typename T>
    T random(T from, T to);

    bool random(double prob) { return std::bernoulli_distribution(prob)(_modelingParameters.randomEngine()); }

    double distanceToWaypointM(const RoutePoint &waypoint) const
    {
        return degToM(distance(waypoint.latitudeDeg(), waypoint.longitudeDeg(), _latitudeDeg, _longitudeDeg));
    }

    double angleToWaypointDeg(const PointCoordinates &waypointCoordinates) const
    {
        return angleFromCoordsDeg(_latitudeDeg, _longitudeDeg, waypointCoordinates.latitudeDeg(),
                                  waypointCoordinates.longitudeDeg());
    }

    void subscribeForCommands()
    {
        //auto msg(mcc::misc::makeCmdSubscribeRequest(_deviceName));
        //_net.sendMessage(msg);
    }

    void unsubscribeFromCommands()
    {
        //auto msg(mcc::misc::makeCmdUnSubscribeRequest(_deviceName));
        //_net.sendMessage(msg);
    }

    void sendRouteNotFound(const std::string& trait, const messages::Cmd & cmd, uint16_t name)
    {
        mcc::misc::sendCommandStateEvent(_out, cmd, CmdState::Failed,
                                         "Маршрут с именем '" + std::to_string(name)+ "' не найден");
    }

    std::vector<Route>::iterator findRouteByName(uint16_t name)
    {
        return find_if(_routes.begin(), _routes.end(), [name](const Route& route){ return route.name() == name; });
    }
};

static inline const std::string modelName(uint8_t modelNum)
{
    switch (modelNum) {
    case 1: return mcc::Names::model1();
    case 2: return mcc::Names::model2();
    case 3: return mcc::Names::model3();
    case 4: return mcc::Names::model4();
    default: return "mcc.unknown.model";
    }
}

class Model1 :public SimpleModel
{
public:
    Model1(const mcc::messages::LocalRouterPtr& router, uint16_t udpPort) : Model1(router, modelName(1), udpPort)
    {
    }

    Model1(const mcc::messages::LocalRouterPtr& router, const std::string & serviceName, uint16_t udpPort)
        : SimpleModel(router, serviceName, serviceName, udpPort)
    {
        setTickS(0.250);
        setSleepMs(250);
    }
};

class Model2 :public SimpleModel
{
public:
    Model2(const mcc::messages::LocalRouterPtr& router, uint16_t udpPort) : Model2(router, modelName(2), udpPort)
    {
    }

    Model2(const mcc::messages::LocalRouterPtr& router, const std::string & serviceName, uint16_t udpPort)
        : SimpleModel(router, serviceName, serviceName, udpPort, 55.973579, 37.412816, 300.,
        Route(2, SimplifiedRouteKind::Auto, true, 0, 50, "Model 2 route", { RoutePoint(55.930206, 37.518173, 100., 60., 1),
        RoutePoint(55.751958, 37.618155, 100., 60.), RoutePoint(55.520834, 37.549276, 200., 60.),
        RoutePoint(55.724031, 37.272329, 300., 60.), RoutePoint(55.916189, 37.846193, 50., 40.),
        RoutePoint(56.31033, 38.130507, 500., 100., 8) }),
        ModelingParameters(.25, 250, { 2 }))
    {
        appendRoute(Route(1, SimplifiedRouteKind::Auto, true, 0, 50, "Route 2",
        {
            RoutePoint(55.8198, 36.9745, 100., 60.),
            RoutePoint(56.0122, 37.4551, 100., 60.),
            RoutePoint(56.1547, 36.6586, 100., 60.),
            RoutePoint(56.3272, 37.1365, 100., 60.)
        }
        ));
    }
};

class Model3 :public SimpleModel
{
public:
    Model3(const mcc::messages::LocalRouterPtr& router, uint16_t udpPort) : Model3(router, modelName(3), udpPort)
    {
    }

    Model3(const mcc::messages::LocalRouterPtr& router, const std::string & serviceName, uint16_t udpPort)
        : SimpleModel(router, serviceName, serviceName, udpPort, 33.944054, -118.413939, 0.0,
        Route(3, SimplifiedRouteKind::Auto, true, 0, 50, "Model 3 route",
        {
            RoutePoint(40.774221, -73.872793, 1000.0, 1200.0, 1),
            RoutePoint(48.728777, 2.365703, 1000.0, 1200.0),
            RoutePoint(55.973552, -118.413939, 1000.0, 1200.0),
            RoutePoint(39.91886, 116.385471, 1200.0, 1200.0),
            RoutePoint(35.72109, 139.690143, 4000.0, 1200.0),
            RoutePoint(33.944054, -118.413939, 4000.0, 1200.0, 8)
        }),
        ModelingParameters(.25, 250, { 3 }),
        DeviceModelingParameters(1200.0, 40.0, 50., 30., true))
    {
    }
};

class Model4 :public SimpleModel
{
public:
    Model4(const mcc::messages::LocalRouterPtr& router, uint16_t udpPort) : Model4(router, modelName(4), udpPort)
    {
    }

    Model4(const mcc::messages::LocalRouterPtr& router, const std::string & serviceName, uint16_t udpPort)
        : SimpleModel(router, serviceName, serviceName, udpPort, 55.75435, 37.622864, 0.0,
        Route(),
        ModelingParameters(.25, 250, { 4 }), DeviceModelingParameters(120.0, 10.0))
    {
        double infStartLatGrad(55.75435), infStartLongGrad(37.622864), radiusGrad(0.1);
        uint8_t segments(20);
        std::vector<RoutePoint> infRoute;
        for (size_t i(0); i < segments; i++) {
            double angleRad(misc::degreesToRadians(180. - i * 360. / segments));
            infRoute.emplace_back(RoutePoint(infStartLatGrad + radiusGrad * std::sin(angleRad),
                infStartLongGrad + radiusGrad + radiusGrad * std::cos(angleRad), 500.0, 60.0));
        }
        for (size_t i(0); i < segments; i++) {
            double angleRad(misc::degreesToRadians(i * 360.0 / segments));
            infRoute.emplace_back(RoutePoint(infStartLatGrad + radiusGrad * std::sin(angleRad),
                infStartLongGrad - radiusGrad + radiusGrad * std::cos(angleRad), 500.0, 60.0));
        }
        appendRoute(Route(4, SimplifiedRouteKind::Auto, true, 0, 50, "Model 4 route", infRoute));
    }
};

}
}
