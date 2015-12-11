/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#pragma once

#include <QObject>
#include <QDebug>

#include <vector>

#include "mcc/ui/core/FlyingDevice.h"

namespace mcc
{
    namespace ui
    {
        namespace core
        {
            class DevicesGroup : public QObject
            {
                Q_OBJECT
            public:
                DevicesGroup(const QString& name)
                    : _name(name)
                {

                }

                void addDevice(FlyingDevice* device)
                {
                    auto it = std::find(_devices.begin(), _devices.end(), device);
                    if (it != _devices.end())
                    {
                        qDebug() << "Device is already in group";
                        return;
                    }

                    _devices.push_back(device);

                    emit deviceAdded(device);
                }

                void removeDevice(FlyingDevice* device)
                {
                    auto it = std::find(_devices.begin(), _devices.end(), device);
                    if (it == _devices.end())
                    {
                        qDebug() << "Device is not exists in group";
                        return;
                    }

                    _devices.erase(it);

                    emit deviceRemoved(device);
                }

                void takeOff()
                {

                }

                void land()
                {

                }

                void moveTo(const GeoPosition& position, const GeoOrientation& orientation)
                {
                    Q_UNUSED(position);
                    Q_UNUSED(orientation);

                    using mcc::misc::Cmd;

                    /*for (auto dev : _devices)
                    {
                    }*/
                }

                std::vector<FlyingDevice*> devices() const
                {
                    return _devices;
                }

                QString name() const
                {
                    return _name;
                }

            signals:
                void deviceAdded(FlyingDevice* device);
                void deviceRemoved(FlyingDevice* device);

            private:
                QString _name;

                std::vector<FlyingDevice*> _devices;
            };
        }
    }
}

