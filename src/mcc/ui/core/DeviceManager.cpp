/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "mcc/ui/core/DeviceManager.h"
#include "mcc/Names.h"

#include <QApplication>
#include <QStyle>
#include <QPixmap>
#include <QDebug>

namespace mcc {
    namespace ui {
        namespace core {

            DeviceManager::DeviceManager()
                : _selectedDevice(nullptr)
                , _offlineMode(false)
            {
                startTimer(1000);

                startExchange();
            }

            FlyingDevice* DeviceManager::currentDevice()
            {
                return _selectedDevice;
            }

            void DeviceManager::processTm(const QDateTime& time, const mcc::misc::TmParam& tmParam)
            {
                Q_UNUSED(time);
                Q_UNUSED(tmParam);
                /*FlyingDevice* device;

                if (!_devices.contains(tmParam.device()))
                {
                    addDevice(tmParam.device());
                }

                device = _devices[tmParam.device()];
                Q_ASSERT(device != nullptr);
                */
                //device->processTm(time, tmParam);
            }

            void DeviceManager::selectDevice(FlyingDevice* device)
            {
                _selectedDevice = device;

                emit(selectionChanged(device));
            }

            void DeviceManager::addDevice(const QString& deviceName, bool setCurrent /*= false*/)
            {
                if (_devices.contains(deviceName))
                {
                    qDebug() << "Device " << deviceName << " already exists. ";
                    return;
                }

                QPixmap devicePixmap;
                if (_devices.count() < _pixmaps.count())
                    devicePixmap = _pixmaps[_devices.count()];
                else
                    devicePixmap = QApplication::style()->standardIcon(QStyle::SP_TrashIcon).pixmap(64, 64);


                FlyingDevice* device = new FlyingDevice(this, deviceName, devicePixmap);

                connect(device, &FlyingDevice::onSendCmd, this, &DeviceManager::sendCmd);
                connect(device, &FlyingDevice::routeUploaded, this, &DeviceManager::endEditRoute);

                _devices[deviceName] = device;

                if (setCurrent)
                    selectDevice(device);

                subscribeAllTm(device);

                emit(deviceAdded(device));
            }

            void DeviceManager::removeDevice(FlyingDevice* device)
            {
                qDebug() << "Removing device " << device->name();
                Q_ASSERT(device != nullptr);

                auto name = device->name();
                if (!_devices.contains(name))
                {
                    Q_ASSERT(false);
                    return;
                }

                FlyingDevice* d = _devices[device->name()];
                delete d;
                _devices.remove(device->name());
            }

            DeviceManager::~DeviceManager()
            {
                for (auto dev : _devices.values())
                {
                    delete dev;
                }
            }

            void DeviceManager::clear()
            {
                for (auto dev : _devices.values())
                {
                    emit deviceRemoved(dev);
                    delete dev;
                }

                _devices.clear();

                selectDevice(nullptr);
            }

            QVector<FlyingDevice*> DeviceManager::devicesList() const
            {
                return _devices.values().toVector();
            }

            void DeviceManager::setPixmapList(const QVector<QPixmap>& pixmaps)
            {
                _pixmaps = pixmaps;
            }

            void DeviceManager::subscribeAllTm(FlyingDevice* device)
            {
                using mcc::misc::TmParam;

                emit(subscribeTmParam(TmParam(device->name(), "Device", "batteryLevel")));
                emit(subscribeTmParam(TmParam(device->name(), "Device", "signalLevel")));

                emit(subscribeTmParam(TmParam(device->name(), "Navigation.Motion", "latitude")));
                emit(subscribeTmParam(TmParam(device->name(), "Navigation.Motion", "longitude")));
                emit(subscribeTmParam(TmParam(device->name(), "Navigation.Motion", "altitude")));
                emit(subscribeTmParam(TmParam(device->name(), "Navigation.Motion", "pitch")));
                emit(subscribeTmParam(TmParam(device->name(), "Navigation.Motion", "roll")));
                emit(subscribeTmParam(TmParam(device->name(), "Navigation.Motion", "heading")));
                emit(subscribeTmParam(TmParam(device->name(), "Navigation.Motion", "throttle")));
                emit(subscribeTmParam(TmParam(device->name(), "Navigation.Route", "nextPoint")));

                for (int i = 1; i <= 6; ++i)
                {
                    auto trait = QString("Route.Point%1").arg(i);
                    emit(subscribeTmParam(TmParam(device->name(), trait, "latitude")));
                    emit(subscribeTmParam(TmParam(device->name(), trait, "longitude")));
                    emit(subscribeTmParam(TmParam(device->name(), trait, "altitude")));
                    emit(subscribeTmParam(TmParam(device->name(), trait, "speed")));
                }

                emit(subscribeTmParam(TmParam(device->name(), "Debug.Waypoint", "Latitude")));
                emit(subscribeTmParam(TmParam(device->name(), "Debug.Waypoint", "Longitude")));
                emit(subscribeTmParam(TmParam(device->name(), "Debug.Waypoint", "LatitudeDelta")));
                emit(subscribeTmParam(TmParam(device->name(), "Debug.Waypoint", "LongitudeDelta")));
                emit(subscribeTmParam(TmParam(device->name(), "Debug.Waypoint", "Angle")));
                emit(subscribeTmParam(TmParam(device->name(), "Debug.Waypoint", "Distance")));
                emit(subscribeTmParam(TmParam(device->name(), "Debug.Waypoint", "AngleDelta")));
            }

            void DeviceManager::sendCmd(const mcc::misc::Cmd& cmd)
            {
                _sentCmds[cmd.collationId()] = cmd;

                emit(sendCmdToExchange(cmd));
            }

            void DeviceManager::processTmParamList(const QDateTime& time, const std::map<QString, mcc::misc::TmParam>& tmParamList)
            {
                if (tmParamList.empty())
                {
                    Q_ASSERT(false);
                    return;
                }

                FlyingDevice* device = nullptr;

                auto deviceName = tmParamList.begin()->second.device();

                if (!_devices.contains(deviceName))
                {
                    return;
                    Q_ASSERT(false);
                    addDevice(deviceName);
                }

                device = _devices[deviceName];

                Q_ASSERT(device != nullptr);

                device->processTmParamList(time, tmParamList);
            }

            QVector<QPixmap> DeviceManager::pixmapList() const
            {
                return _pixmaps;
            }

            void DeviceManager::timerEvent(QTimerEvent *)
            {
                if (_offlineMode)
                    return;

                const int INACTIVE_STATE = 10;

                QDateTime currentDateTime = QDateTime::currentDateTime();

                for (auto device : _devices)
                {
                    if (device->lastTmMsgDateTime().isNull())
                    {
                        QDateTime pastDateTime = currentDateTime.addSecs(-INACTIVE_STATE - 1);

                        device->setLastMsgTime(pastDateTime);
                        device->setActive(false);
                        device->setSignalBad();
                        emit(deviceSignalBad(device));
                        continue;
                    }

                    int inactiveSecs = device->lastTmMsgDateTime().secsTo(currentDateTime);
                    if ((inactiveSecs > INACTIVE_STATE) && device->isActive())
                    {
                        device->setActive(false);
                        device->setSignalBad();
                        emit(deviceSignalBad(device));
                    }

                    if ((inactiveSecs <= INACTIVE_STATE) && !device->isActive())
                    {
                        device->setActive(true);
                        device->setSignalGood();
                        emit(deviceSignalGood(device));
                    }

                    if (!device->isStateActive() && _devicesForExchange.contains(device->name()))
                    {
                        emit deviceNotReadyForExchange(device);
                        _devicesForExchange.removeAt(_devicesForExchange.indexOf(device->name()));
                    }
                }

                QVector<QString> aliveDevices;
                for (const auto& devicesList : _devicesInChannels)
                {
                    for (const auto& device : devicesList)
                    {
                        aliveDevices.append(device);
                    }
                }

                QVector<QString> deadDevices;
                for (auto& deviceName : _devices.keys())
                {
                    if (!aliveDevices.contains(deviceName))
                    {
                        deadDevices.append(deviceName);
                    }
                }

                for (auto& deadName : deadDevices)
                {
                    FlyingDevice* device = _devices[deadName];
                    emit deviceRemoved(device);
                    _devices.remove(deadName);

                    // после рефакторинга можно будет удалять и сам указатель. пока пусть течет память, благо её немного
                    //                    delete device;

                    if (_devicesForExchange.contains(deadName))
                    {
                        emit deviceNotReadyForExchange(device);
                        _devicesForExchange.removeAt(_devicesForExchange.indexOf(deadName));

                        qDebug() << QString("Device %1 not ready for exchange").arg(deadName);
                    }
                }
            }

            mcc::misc::Cmd DeviceManager::getSentCmd(mcc::misc::CmdCollationId collationId) const
            {
                if (!_sentCmds.contains(collationId))
                {
                    //HACK: непонятно, что делать с таким количеством спамящих команд
                    return mcc::misc::Cmd(collationId, "mcc.device", QString(), QString());
                }

                return _sentCmds[collationId];
            }

            void DeviceManager::onDeviceRegistered(std::size_t id, const QString& name, const QString& info)
            {
                emit requestDeviceDescription(name);
            }

            void DeviceManager::onDeviceState(const QString& service, const mcc::misc::DeviceStateList & statesList)
            {
                if (!_devicesInChannels.contains(service))
                    _devicesInChannels[service] = DevicesList();

                for(auto deviceState : statesList)
                {
                    if (!_devices.contains(deviceState._device))
                    {
                        addDevice(deviceState._device);

                        FlyingDevice* dev = _devices[deviceState._device];
                        if (dev == nullptr)
                        {
                            Q_ASSERT(false);
                            qDebug() << "Unknown device: " << deviceState._device;

                            continue;
                        }

//                        if (deviceState._isRegistered && dev->traits().empty())
                            emit requestDeviceDescription(deviceState._device);

                        _devicesInChannels[service].append(deviceState._device);
                    }

                    _devices[deviceState._device]->setDeviceState(deviceState);

                    if (deviceState._isActive && deviceState._isRegistered && !_devicesForExchange.contains(deviceState._device))
                    {
                        emit deviceReadyForExchange(_devices[deviceState._device]);
                        _devicesForExchange.append(deviceState._device);
                        qDebug() << QString("Device %1 ready for exchange").arg(deviceState._device);
                    }
                }

                // удаление несуществующих ссылок на устройства в каналах
                auto channelContainsDeviceFunc = [&](const QString& device) {
                    auto it = std::find_if(statesList.begin(), statesList.end(),
                        [=](const mcc::misc::DeviceState& dev)
                        {
                            return dev._device == device;
                        }
                    );
                    return it != statesList.end();
                };


                auto eraseIt = std::remove_if(_devicesInChannels[service].begin(), _devicesInChannels[service].end(), [&](const QString& dev){ return !channelContainsDeviceFunc(dev); });

                _devicesInChannels[service].erase(eraseIt, _devicesInChannels[service].end());
            }

            void DeviceManager::onDeviceDescription(const mcc::misc::DeviceDescription& device)
            {
                if (!_devices.contains(device._device_name))
                {
//                    Q_ASSERT(false);
//                    qDebug() << "Unknown device: " << device._device_name;
                    return;
                }

                _devices[device._device_name]->setDeviceDescription(device);

                emit requestFirmwareDescription(device._firmware_name);
                emit requestProtocolForDevice(device._device_name);
            }

            void DeviceManager::onFirmwareDescription(const QString& firmware, const mcc::misc::TraitDescriptionList& traits)
            {
                FlyingDevice* device = nullptr;

                for (auto dev : _devices.values())
                {
                    if (dev->deviceDescription()._firmware_name == firmware)
                    {
                        device = dev;

                        if (device->traits().empty())
                        {
                            device->setTraits(traits);
                            emit deviceFirmwareLoaded(device);
                        }
                    }
                }

                if (device == nullptr)
                {
                    qDebug() << "Unknown firmware: " << firmware;
                    return;
                }
            }

            void DeviceManager::cancelCmd(const QString& device, mcc::misc::CmdCollationId collationId)
            {
                emit requestCancelCmd(device, collationId);
            }

            void DeviceManager::startReadingParam(const QString& device, const QString& trait, int firstParamId, int paramCount)
            {
                sendCmd(mcc::misc::Cmd(
                    mcc::misc::Cmd::generateCollationId(),
                    device,
                    "Tm",
                    "startReading",
                    { trait, firstParamId, paramCount }
                    ));

                QString alias = QString("%1.%2.%3").arg(device).arg(trait).arg(firstParamId);
                if (paramCount == 1 && !_protocolParams.contains(alias))
                {
                    _protocolParams[alias] = ProtocolParamDescriptor(device, trait, firstParamId);
                }
            }

            void DeviceManager::stopReadingParam(const QString& device, const QString& trait, int firstParamId, int paramCount)
            {
                sendCmd(mcc::misc::Cmd(
                    mcc::misc::Cmd::generateCollationId(),
                    device,
                    "Tm",
                    "stopReading",
                    {trait, firstParamId, paramCount}
                    ));

                // если отписываемся от 1 переменной - то просто отписываемся и удаляем её дескриптор
                QString alias = QString("%1.%2.%3").arg(device).arg(trait).arg(firstParamId);
                if (paramCount == 1)
                {
                    if (_protocolParams.contains(alias))
                        _protocolParams.remove(alias);

                    return;
                }

                // в случае, если мы отписываемся от узла, но при этом у нас есть подписка на отдельные переменные в этом узле
                // - переподписываемся на эти переменные

                QVector<ProtocolParamDescriptor> paramsToRestore;

                std::for_each(_protocolParams.begin(), _protocolParams.end(),
                    [&](const ProtocolParamDescriptor& desc)
                    {
                        if(desc._device == device && desc._trait == trait)
                            paramsToRestore.append(desc);
                    }
                );

                for (const auto& it : paramsToRestore)
                {
                    startReadingParam(it._device, it._trait, it._paramId, 1);
                }

            }

            void DeviceManager::onProtocolForDevice(const QString& device, const QVector<mcc::misc::ProtocolId>& id)
            {
                if (!_devices.contains(device))
                {
                    //                    Q_ASSERT(false);
                    //qDebug() << "Unknown device: " << device;
                    return;
                }

                if (id.empty())
                    return;

                _devices[device]->setProtocolId(id.first());
            }

            bool DeviceManager::readyForExchange(const QString& device) const
            {
                return _devicesForExchange.contains(device) && _devices[device]->isStateActive();
            }

            void DeviceManager::createFakeDevice()
            {
                addDevice("БПЛА", true);

                auto device = currentDevice();
                device->addRoute(new Route("Маршрут", 1));
            }

            void DeviceManager::startExchange()
            {
                _offlineMode = false;
            }

            void DeviceManager::stopExchange()
            {
                _offlineMode = true;
            }

            bmcl::Option<FlyingDevice*> DeviceManager::device(const QString& name)
            {
                if (_devices.contains(name))
                    return _devices[name];

                return bmcl::None;
            }


            bmcl::Option<FlyingDevice*> DeviceManager::readyForExchangeWithProtocolId(const mcc::misc::ProtocolId& id)
            {
                FlyingDevice* dev = nullptr;
                for (auto d : _devices)
                {
                    if (d->protocolId().id() == id.id())
                    {
                        dev = d;
                        break;
                    }
                }

                if (dev == nullptr)
                    return bmcl::None;

                if (_devicesForExchange.contains(dev->name()) && _devices[dev->name()]->isStateActive())
                    return dev;

                return bmcl::None;
            }

            DevicesGroup* DeviceManager::addGroup(const QString& name)
            {
                auto it = std::find_if(_devicesGroups.begin(), _devicesGroups.end(), [=](DevicesGroup* group) { return group->name() == name; });
                if (it != _devicesGroups.end())
                {
                    qDebug() << "Group already exists";
                    return *it;
                }

                DevicesGroup* group = new DevicesGroup(name);

                _devicesGroups.push_back(group);

                emit groupAdded(group);

                return group;
            }

            void DeviceManager::removeGroup(DevicesGroup* group)
            {
                auto it = std::find(_devicesGroups.begin(), _devicesGroups.end(), group);
                if (it == _devicesGroups.end())
                {
                    qDebug() << "Group is not exists";
                    return;
                }

                _devicesGroups.erase(it);

                emit groupRemoved(group);
            }

        }
    }
}
