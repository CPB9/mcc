/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#pragma once

#include <QTreeWidget>

#include "mcc/misc/TmParam.h"

#include "mcc/ui/flightcontrols/DeviceStateWidget.h"

#include "mcc/ui/core/DeviceManager.h"

namespace mcc {
    namespace ui {
        namespace flightcontrols {

            class DevicesListWidget : public QTreeWidget
            {
                Q_OBJECT

            public:
                explicit DevicesListWidget(QWidget *parent = 0);
                ~DevicesListWidget();

                void setDeviceManager(mcc::ui::core::DeviceManager* manager);
            signals:
                void deviceSelected(mcc::ui::core::FlyingDevice* device);
                void deviceCentered(mcc::ui::core::FlyingDevice* device);
                void requestDeviceActivate(const QString& device, bool isActive);

            public slots:
                void addDevice(mcc::ui::core::FlyingDevice* aircraft);
                void removeDevice(mcc::ui::core::FlyingDevice* aircraft);

                void addGroup(mcc::ui::core::DevicesGroup* group);
                void removeGroup(mcc::ui::core::DevicesGroup* group);

                void deviceSignalGood(mcc::ui::core::FlyingDevice* device);
                void deviceSignalBad(mcc::ui::core::FlyingDevice* device);
                void selectDevice(mcc::ui::core::FlyingDevice* device);

                void createContextMenu(const QPoint & pos);
            private:
                void addDeviceWidget(mcc::ui::core::FlyingDevice* device, QTreeWidgetItem* parent);
                void removeDeviceWidget(mcc::ui::core::FlyingDevice* device, QTreeWidgetItem* parent);

                mcc::ui::core::DeviceManager*     _manager;
                QTreeWidgetItem*                  _ungroupedDevicesNode;
            };
        }
    }
}
