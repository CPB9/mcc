/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#pragma once

#include <QObject>
#include <QPixmap>
#include <QTime>
#include <QMap>
#include <QMetaType>

#include "mcc/ui/core/Structs.h"
#include "mcc/ui/core/Route.h"

#include "mcc/misc/TmParam.h"
#include "mcc/misc/Cmd.h"
#include "mcc/misc/Device.h"
#include "mcc/misc/Firmware.h"
#include "mcc/misc/Protocol.h"

namespace mcc {
    namespace ui {
        namespace core{

            class DeviceManager;

            class FlyingDevice : public QObject
            {
                Q_OBJECT

            public:
                FlyingDevice(DeviceManager* manager, const QString& name, const QPixmap& pixmap);
                ~FlyingDevice();

                DeviceManager*          manager()       const { return _manager;         }
                const QString&          name()          const { return _name;            }
                const QPixmap&          pixmap()        const { return _pixmap;          }
                const GeoPosition&      position()      const { return _lastPosition;    }
                const GeoPosition&      dogPosition()   const { return _lastDogPosition; }
                const GeoOrientation&   orientation()   const { return _lastOrientation; }

                Route*                  activeRoute()               { return findRouteById(_currentRouteId);     }
                Route*                  selectedRoute()             { return findRouteById(_selectedRouteId);    }
                void                    setSelectedRoute(int routeId)
                {
                    _selectedRouteId = routeId;

                    auto route = selectedRoute();
                    Q_ASSERT(route != nullptr);

                    emit routeSelectionChanged(route);
                }

                int                     activeRouteId() const { return _currentRouteId; }

                double                  speed()         const { return _speed;           }
                double                  targetHeading() const { return _targetHeading;   }
                int                     nextWaypoint()  const { return _nextWaypoint;    }
                double                  accuracy()      const { return _accuracy;        }
                void                    setPixmap(const QPixmap& pixmap) { _pixmap = pixmap; emit pixmapChanged(_pixmap); }

                QString                 currentMode()   const;

                int battery()           const { return _battery; }
                int signal()            const { return _signal; }
                int throttle()          const { return _throttle; }

                QTime lastTmMsgTime()   const { return  _lastTmMsgDateTime.time(); }
                const QDateTime& lastTmMsgDateTime()   const { return  _lastTmMsgDateTime; }
                bool isActive()         const { return _isActive; }

                void setActive(bool active) { _isActive = active; }
                void setSignalGood() { emit signalGood(); };
                void setSignalBad() { emit signalBad(); };

                bool isStateActive() { return _deviceState._isActive; }

                mcc::ui::core::TrailMode trailMode() const { return _trailMode; }
                int trailCount() const { return _trailCount; }
                void setTrailMode(mcc::ui::core::TrailMode mode);
                void setTrailCount(int count);
                void clearTail();

                const mcc::misc::DeviceState& deviceState() const { return _deviceState; }
                void setDeviceState(const mcc::misc::DeviceState& state);

                const mcc::misc::DeviceDescription& deviceDescription() const;
                void setDeviceDescription(const mcc::misc::DeviceDescription& deviceDescription);

                const mcc::misc::TraitDescriptionList& traits() const;
                void setTraits(const mcc::misc::TraitDescriptionList& traits);

                const QVector<Route*>& routes() const;

                void addRoute(Route* route);
                Route* findRouteById(int name);
                void setActiveRoute(Route* route);
                void setNextWaypoint(Route* route, int index);

                void createRoute(const QString& name);
                void removeRoute(const QString& name);

                QString state1() const { return _state1; }
                QString state2() const { return _state2; }

                mcc::misc::ProtocolId protocolId() const { return _protocolId; };
                void setProtocolId(const mcc::misc::ProtocolId& protocolId)  { _protocolId = protocolId; }
            signals:
                void motionChanged(const QDateTime& dateTime, const GeoPosition& position, const GeoOrientation& orientation);
                void positionChanged     (const GeoPosition& position);
                void dogPositionChanged  (const GeoPosition& position);
                void orientationChanged  (const GeoOrientation& orientation);
                void accuracyChanged(double accuracy);

                void pixmapChanged       (const QPixmap& pixmap);
                void speedChanged        (double airSpeed);
                void nextWaypointChanged (int index);
                void tmParamChanged      (const QDateTime& time, const mcc::misc::TmParam& tmParam);
                void targetHeadingChanged(double heading);
                void batteryChanged      (int battery);
                void signalChanged       (int signal);
                void throttleChanged     (int throttle);

                void onSendCmd           (const mcc::misc::Cmd& cmd);
                void routeUploaded();

                void trailModeChanged    (mcc::ui::core::TrailMode mode);
                void trailCountChanged   (int count);
                void trailModeAndCountChanged(mcc::ui::core::TrailMode mode, int count);
                void trailCleared();
                void deviceStateChanged();
                void signalGood();
                void signalBad();

                void routeAdded(Route* route);
                void routeRemoved(Route* route);
                void routeSelectionChanged(Route* route);

                void activeRouteChanged(Route* route);
                void stateChanged(const QString& state1, const QString& state2);
                void deviceDescriptionUpdated(const mcc::misc::DeviceDescription& deviceDescription);

            public slots:
                virtual void processTmParamList(const QDateTime& time, const std::map<QString, mcc::misc::TmParam>& tmParamList);

                void processRoutes(const std::map<QString, mcc::misc::TmParam>&pMap);

                void setLastMsgTime(const QDateTime &time);

                void sendCmd(const mcc::misc::Cmd& cmd);

                void resetEditableRoute(int routeIdx = 0);
                void uploadEditableRoute();
            private:
                double getDoubleParam(const std::map<QString, mcc::misc::TmParam>& params, const QString& trait);
                unsigned int getUintParam(const std::map<QString, mcc::misc::TmParam>& params, const QString& trait);
                QString getStringParam(const std::map<QString, mcc::misc::TmParam>& params, const QString& trait);
            protected:
                QString         _name;
                QPixmap         _pixmap;
                QString         _state1;
                QString         _state2;
                DeviceManager*  _manager;
                QDateTime       _lastTmMsgDateTime;
                size_t          _rcvdPackets;

                bool            _isActive;

                unsigned int    _activeMode;
                GeoPosition     _lastPosition;
                GeoPosition     _lastDogPosition;
                GeoOrientation  _lastOrientation;

                int               _currentRouteId;
                int               _selectedRouteId;

                QVector<Route*>   _routes;

                double          _speed;
                double          _targetHeading;
                unsigned int    _battery;
                unsigned int    _signal;
                unsigned int    _throttle;
                int             _nextWaypoint;
                double          _accuracy;

                mcc::ui::core::TrailMode _trailMode;
                int                      _trailCount;

                QMap<int, QString> _modeConsts;
                mcc::misc::DeviceState _deviceState;
                mcc::misc::DeviceDescription _deviceDescription;
                mcc::misc::TraitDescriptionList _traits;
                mcc::misc::ProtocolId _protocolId;
                QDateTime _lastPosTime;
                QDateTime _lastLeadTime;
                QDateTime _lastOrientationTime;
            };
        }
    }
}

Q_DECLARE_METATYPE(mcc::ui::core::FlyingDevice*);
