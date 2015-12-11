/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#pragma once

#include <QDialog>

#include "mcc/ui/flightcontrols/ui_DeviceSettingsWidget.h"

#include "mcc/ui/core/FlyingDevice.h"

namespace mcc {
    namespace ui {
        namespace flightcontrols {

            class DeviceSettingsWidget : public QDialog
            {
                Q_OBJECT

            public:
                explicit DeviceSettingsWidget(QWidget *parent = 0);

                void setDevice(mcc::ui::core::FlyingDevice* device);

                int trailCount() const;
                mcc::ui::core::TrailMode trailMode() const;

                bool clearTrail() const;

                QPixmap pixmap() const;
            private:
                Ui::DeviceSettingsWidget     _ui;
                mcc::ui::core::FlyingDevice* _device;
            };

        }
    }
}
