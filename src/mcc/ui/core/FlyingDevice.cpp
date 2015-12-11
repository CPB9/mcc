/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "mcc/ui/core/FlyingDevice.h"
#include "mcc/ui/core/DeviceManager.h"

#include <QStringList>
#include <QMap>
#include <QDebug>
#include <QDateTime>
#include <bmcl/Logging.h>

#define SET_IF_CHANGED_DOUBLE(oldValue, newValue) \
    if (!doubleEq(oldValue, newValue)) \
            { \
        oldValue = newValue; \
            };


#define SET_IF_CHANGED_DOUBLE_AND_EMIT(oldValue, newValue, signal) \
    if (!doubleEq(oldValue, newValue)) \
    { \
        oldValue = newValue; \
        emit signal(oldValue); \
    };

#define SET_IF_CHANGED(oldValue, newValue) \
    if (oldValue != newValue) \
                    { \
        oldValue = newValue; \
                    };


#define SET_IF_CHANGED_AND_EMIT(oldValue, newValue, signal) \
    if (oldValue != newValue) \
            { \
        oldValue = newValue; \
        emit signal(oldValue); \
            };

namespace mcc {
    namespace ui {
        namespace core {

            FlyingDevice::FlyingDevice(DeviceManager* manager, const QString& name, const QPixmap& pixmap)
                : _name(name)
                , _pixmap(pixmap)
                , _manager(manager)
                , _rcvdPackets(0)
                , _isActive(false)
                , _activeMode(0)
                , _currentRouteId(0)
                , _selectedRouteId(-1)
                , _speed(0.0)
                , _targetHeading(0.0)
                , _battery(0)
                , _signal(0)
                , _throttle(0)
                , _nextWaypoint(0)
                , _accuracy(0.0)
            {
                _modeConsts[0] = "Выключено";
                _modeConsts[1] = "Готовность ПН";
                _modeConsts[2] = "Готовность ДВС";
                _modeConsts[3] = "Взлет";
                _modeConsts[4] = "Свободный полёт";
                _modeConsts[5] = "Полёт по маршруту";
                _modeConsts[6] = "Режим ожидания";
                _modeConsts[7] = "Полёт домой";
                _modeConsts[8] = "Посадка";

                _trailMode = TrailMode::Distance;
                _trailCount = 2000;
            }

            FlyingDevice::~FlyingDevice()
            {
                for (Route* route : _routes) {
                    delete route;
                }
            }


            void FlyingDevice::sendCmd(const mcc::misc::Cmd& cmd)
            {
                emit(onSendCmd(cmd));
            }

            void FlyingDevice::processTmParamList(const QDateTime& time, const std::map<QString, mcc::misc::TmParam>& params)
            {
                unsigned int activeMode = getUintParam(params, "Navigation.Mode.activeMode");

                Q_UNUSED(activeMode)

                quint64 timeMsecsSinceEpoch = time.toMSecsSinceEpoch();

                if (params.find("Navigation.Motion.latitude") != params.end())
                {
                    GeoPosition currentPosition;
                    currentPosition.latitude = getDoubleParam(params, "Navigation.Motion.latitude");
                    currentPosition.longitude = getDoubleParam(params, "Navigation.Motion.longitude");
                    currentPosition.altitude = getDoubleParam(params, "Navigation.Motion.altitude");

                    if (currentPosition.isValid() && (timeMsecsSinceEpoch - _lastPosTime.toMSecsSinceEpoch()) > 200)
                    {
                        SET_IF_CHANGED_AND_EMIT(_lastPosition, currentPosition, positionChanged);
                        _lastPosTime = time;
                    }
                }

                if (params.find("Navigation.DogPoint.latitude") != params.end())
                {
                    GeoPosition leadPosition;
                    leadPosition.latitude = getDoubleParam(params, "Navigation.DogPoint.latitude");
                    leadPosition.longitude = getDoubleParam(params, "Navigation.DogPoint.longitude");
                    leadPosition.altitude = getDoubleParam(params, "Navigation.DogPoint.altitude");

                    if (leadPosition.isValid() && (timeMsecsSinceEpoch - _lastLeadTime.toMSecsSinceEpoch()) > 200)
                    {
                        SET_IF_CHANGED_AND_EMIT(_lastDogPosition, leadPosition, dogPositionChanged);
                        _lastLeadTime = time;
                    }
                }

                if (params.find("Navigation.Motion.heading") != params.end())
                {
                    GeoOrientation currentOrientation;
                    currentOrientation.heading = getDoubleParam(params, "Navigation.Motion.heading");
                    currentOrientation.pitch = getDoubleParam(params, "Navigation.Motion.pitch");
                    currentOrientation.roll = getDoubleParam(params, "Navigation.Motion.roll");

                    if (timeMsecsSinceEpoch - _lastOrientationTime.toMSecsSinceEpoch() > 100)
                    {
                        SET_IF_CHANGED_AND_EMIT(_lastOrientation, currentOrientation, orientationChanged);
                        _lastOrientationTime = time;
                    }
                }


                if (params.find("Navigation.Motion.speed") != params.end())
                {
                    double currentSpeed = getDoubleParam(params, "Navigation.Motion.speed");
                    SET_IF_CHANGED_DOUBLE_AND_EMIT(_speed, currentSpeed, speedChanged);
                }

                if (params.find("Device.batteryLevel") != params.end())
                {
                    unsigned int battery = getUintParam(params, "Device.batteryLevel");
                    SET_IF_CHANGED_AND_EMIT(_battery, battery, batteryChanged);
                }

//                 if (pMap.contains("Device.signalLevel"))
//                 {
//                     unsigned int signal = getUintParam(pMap, "Device.signalLevel");
//                     SET_IF_CHANGED_AND_EMIT(_signal, signal, signalChanged);
//                 }

                if (params.find("Navigation.Motion.throttle") != params.end())
                {
                    unsigned int throttle = getUintParam(params, "Navigation.Motion.throttle");
                    SET_IF_CHANGED_AND_EMIT(_throttle, throttle, throttleChanged);
                }

                if (params.find("Navigation.Motion.accuracy") != params.end())
                {
                    double accuracy = getDoubleParam(params, "Navigation.Motion.accuracy");
                    SET_IF_CHANGED_DOUBLE_AND_EMIT(_accuracy, accuracy, accuracyChanged);
                }

                processRoutes(params);

                emit motionChanged(time, _lastPosition, _lastOrientation);

                auto deviceState1 = params.find("Device.state1");
                auto deviceState2 = params.find("Device.state2");

                if (deviceState1 != params.end() && deviceState2 != params.end())
                {
                    if (_state1 != deviceState1->second.value().qstringify() || _state2 != deviceState2->second.value().qstringify())
                    {
                        _state1 = deviceState1->second.value().qstringify();
                        _state2 = deviceState2->second.value().qstringify();

                        emit stateChanged(_state1, _state2);
                    }
                }

                for (const auto& tmParam : params)
                {
                    emit tmParamChanged(time, tmParam.second);
                }

                if (params.find("Navigation.Mode.activeMode") != params.end())
                {
                    unsigned int mode = getUintParam(params, "Navigation.Mode.activeMode");
                    if (mode == _activeMode)
                        return;

                    auto it = _modeConsts.find(mode);
                    if (it != _modeConsts.end())
                    {
                        _state1 = it.value();
                    }
                    else
                    {
                        _state1 = "Неизвестный режим";
                    }

                    _activeMode = mode;

                    emit stateChanged(_state1, _state2);
                }
            }

            void FlyingDevice::resetEditableRoute(int routeIdx)
            {
                auto srcRoute = std::find_if(_routes.begin(), _routes.end(), [=](Route* r) { return r->id() == routeIdx; });

                if (srcRoute == _routes.end())
                {
                    Q_ASSERT(false);
                    return;
                }
                (*srcRoute)->resetBuffer();
            }

            void FlyingDevice::uploadEditableRoute()
            {
                using mcc::misc::Cmd;

                auto srcRoute = std::find(_routes.begin(), _routes.end(), selectedRoute());

                if (srcRoute == _routes.end())
                {
                    BMCL_WARNING() << "Неизвестный маршрут";
                    return;
                }

                Route* newRoute = (*srcRoute)->buffer();

                QString routeId = QString::number(newRoute->id());

                Cmd cmd(Cmd::generateCollationId(), _name, "Navigation.Routes", "clearRoute", { routeId });
                _manager->sendCmd(cmd);

                int isRing = newRoute->ring() ? 1 : 0;

                cmd = Cmd(Cmd::generateCollationId(), _name, "Navigation.Routes", "beginRoute", { routeId });
                _manager->sendCmd(cmd);

                const WaypointsList& waypoints = newRoute->waypointsList();
                for (int i = 0; i < waypoints.size(); ++i)
                {
                    Waypoint wp = waypoints.at(i);

                    mcc::misc::CmdParams params = {routeId, wp.position.latitude, wp.position.longitude, wp.position.altitude, wp.speed, static_cast<int>(wp.flags)};
                    cmd = Cmd(Cmd::generateCollationId(), _name, "Navigation.Routes", "addRoutePoint", params);
                    _manager->sendCmd(cmd);
                }

                cmd = Cmd(Cmd::generateCollationId(), _name, "Navigation.Routes", "endRoute", {routeId, waypoints.count(), isRing});
                _manager->sendCmd(cmd);

                emit(routeUploaded());
            }

            void FlyingDevice::setTrailMode(mcc::ui::core::TrailMode mode)
            {
                if (mode != _trailMode)
                {
                    _trailMode = mode;
                    emit trailModeChanged(mode);
                    emit trailModeAndCountChanged(mode, _trailCount);
                }
            }

            void FlyingDevice::setTrailCount(int count)
            {
                if (count != _trailCount)
                {
                    _trailCount = count;
                    emit trailCountChanged(count);
                    emit trailModeAndCountChanged(_trailMode, count);
                }
            }

            void FlyingDevice::clearTail()
            {
                emit trailCleared();
            }

            double FlyingDevice::getDoubleParam(const std::map<QString, mcc::misc::TmParam>& params, const QString& trait)
            {
                auto it = params.find(trait);

                if (it != params.end())
                    return it->second.value().toDouble();

                return 0.0;
            }

            unsigned int FlyingDevice::getUintParam(const std::map<QString, mcc::misc::TmParam>& params, const QString& trait)
            {
                auto it = params.find(trait);

                if (it != params.end())
                    return it->second.value().toUint();

                return 0;
            }

            QString FlyingDevice::getStringParam(const std::map<QString, mcc::misc::TmParam>& params, const QString& trait)
            {
                auto it = params.find(trait);

                if (it != params.end())
                    return it->second.value().qstringify();

                return QString();
            }

            void FlyingDevice::setLastMsgTime(const QDateTime &time)
            {
                _lastTmMsgDateTime = time;
            }

            void FlyingDevice::processRoutes(const std::map<QString, mcc::misc::TmParam>& pMap)
            {
                if (pMap.find("Navigation.Routes.count") == pMap.end())
                    return;

                int routesCount = getUintParam(pMap, "Navigation.Routes.count");

                int routeId = getUintParam(pMap, "Navigation.Routes.activeRoute");

                QVector<int> processedRoutes;
                for (int i = 0; i < routesCount; ++i)
                {
                    QString routePrefix = QString("Navigation.Routes.Route%1").arg(i);

                    int     routeId = getUintParam(pMap, QString("%1.name").arg(routePrefix));
                    QString crc = getStringParam(pMap, QString("%1.crc16").arg(routePrefix));

                    Route* route = findRouteById(routeId);
                    if (route && route->crc() == crc)
                        continue;
                    else if (route == nullptr)
                    {
                        QString routeInfo = getStringParam(pMap, QString("%1.info").arg(routePrefix));

                        route = new Route(routeInfo, routeId);
                        addRoute(route);
                    }
                    WaypointsList waypoints;
                    int pointsCount = getUintParam(pMap, QString("%1.pointsCount").arg(routePrefix));
                    for (int j = 0; j < pointsCount; ++j)
                    {
                        QString pointTrait = QString("%1.Point%2").arg(routePrefix).arg(j);

                        double latitude     = getDoubleParam(pMap, pointTrait + ".latitude");
                        double longitude    = getDoubleParam(pMap, pointTrait + ".longitude");
                        double altitude     = getDoubleParam(pMap, pointTrait + ".altitude");
                        double speed        = getDoubleParam(pMap, pointTrait + ".speed");
                        quint32 flags       = getUintParam(pMap, pointTrait + ".flags");

                        waypoints.append(Waypoint(j, latitude, longitude, altitude, speed, static_cast<WaypointType>(flags)));
                    }

                    QString isRingStr = getStringParam(pMap, QString("%1.isRing").arg(routePrefix));
                    bool isRing = (isRingStr == "1") ? true : false;

                    route->setWaypoints(waypoints, crc);
                    route->setRing(isRing);

                    if (route->buffer()->waypointsCount() == 0)
                        route->resetBuffer();
                }

                int nextPoint = getUintParam(pMap, "Navigation.Routes.nextPoint");

                Route* route = activeRoute();
                if (route && _nextWaypoint != nextPoint)
                {
                    _nextWaypoint = nextPoint;
                    emit nextWaypointChanged(nextPoint);
                }

                if (_currentRouteId != routeId)
                {
                    _currentRouteId = routeId;
                    auto route = findRouteById(routeId);
                    emit activeRouteChanged(route);

                    if (route)
                        emit nextWaypointChanged(nextPoint);


                    for (auto route : _routes)
                    {
                        if (route == activeRoute())
                        {
                            QPen pen = route->pen();
                            pen.setWidth(3);

                            route->setStyle(pen, Qt::red, Qt::blue, true);
                        }
                        else
                        {
                            QPen pen = route->pen();
                            pen.setWidth(1);

                            route->setStyle(pen, Qt::blue, Qt::blue, false);
                        }
                    }
                }
            }

            QString FlyingDevice::currentMode() const
            {
                if (_modeConsts.contains(_activeMode))
                    return _modeConsts[_activeMode];
                return QString("Неизв. режим: %1").arg(_activeMode);
            }

            void FlyingDevice::setDeviceState(const mcc::misc::DeviceState& state)
            {
                _deviceState = state;

                if (state._isActive && state._stats._rcvdPackets > _rcvdPackets)
                {
                    setLastMsgTime(state._stats._rcvd);
                    _rcvdPackets = state._stats._rcvdPackets;
                }

                emit deviceStateChanged();
            }

            const mcc::misc::TraitDescriptionList& FlyingDevice::traits() const
            {
                return _traits;
            }

            void FlyingDevice::setTraits(const mcc::misc::TraitDescriptionList& traits)
            {
                _traits = traits;
            }

            const mcc::misc::DeviceDescription& FlyingDevice::deviceDescription() const
            {
                return _deviceDescription;
            }

            void FlyingDevice::setDeviceDescription(const mcc::misc::DeviceDescription& deviceDescription)
            {
                _deviceDescription = deviceDescription;
                emit deviceDescriptionUpdated(_deviceDescription);
            }

            const QVector<Route*>& FlyingDevice::routes() const
            {
                return _routes;
            }

            void FlyingDevice::addRoute(Route* route)
            {
                static int counter = 1;

                const char *colors[] =
                {
                    "LightSalmon",
                    "SteelBlue",
//                    "Yellow",
                    "Fuchsia",
                    //"PaleGreen",
                    //"PaleTurquoise",
                    //"Cornsilk",
                    "HotPink",
                    "Peru",
                    "Maroon"
                };
                const int numColors = sizeof(colors) / sizeof(colors[0]);

                QPen pen;
                pen.setColor(QColor(colors[counter % numColors]));
                pen.setWidth(2);

                route->setStyle(pen, Qt::red, Qt::blue, true);

                counter++;

                qDebug() << QString("Route %1::%2 added").arg(_name).arg(route->name()) << this;

                _routes.append(route);

                if (_selectedRouteId == -1)
                    _selectedRouteId = route->id();

                emit routeAdded(route);
            }

            Route* FlyingDevice::findRouteById(int id)
            {
                auto result = std::find_if(_routes.begin(), _routes.end(), [&](const Route* route){ return route->id() == id; });

                if (result == _routes.end())
                    return nullptr;

                return *result;
            }

            void FlyingDevice::setActiveRoute(Route* route)
            {
                int idx = _routes.indexOf(route);
                if (idx == -1)
                {
                    Q_ASSERT(false);
                    qDebug() << "Неверный маршрут";

                    return;
                }

                using mcc::misc::Cmd;
                sendCmd(Cmd(Cmd::generateCollationId(), _name, "Navigation.Routes", "setActiveRoute", { route->id(), 0 }));
            }

            void FlyingDevice::setNextWaypoint(Route* route, int index)
            {
                using mcc::misc::Cmd;
                sendCmd(Cmd(Cmd::generateCollationId(), _name, "Navigation.Routes", "setActivePoint", { index }));
            }

            void FlyingDevice::createRoute(const QString& name)
            {
                using mcc::misc::Cmd;
                sendCmd(Cmd(Cmd::generateCollationId(), _name, "Navigation.Routes", "createRoute", { name }));
            }

            void FlyingDevice::removeRoute(const QString& name)
            {
                using mcc::misc::Cmd;
                sendCmd(Cmd(Cmd::generateCollationId(), _name, "Navigation.Routes", "deleteRoute", { name }));
            }

        }
    }
}
