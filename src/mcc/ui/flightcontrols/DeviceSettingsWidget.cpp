/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "DeviceSettingsWidget.h"

#include "mcc/ui/core/FlyingDevice.h"
#include "mcc/ui/core/DeviceManager.h"

#include <QIcon>
#include <QListWidget>

namespace mcc {
    namespace ui {
        namespace flightcontrols {

            DeviceSettingsWidget::DeviceSettingsWidget(QWidget *parent /*= 0*/)
            {
                Q_UNUSED(parent);
                _ui.setupUi(this);

                connect(_ui.distanceRadio, &QRadioButton::toggled, _ui.distance, &QLineEdit::setEnabled);
                connect(_ui.timeRadio, &QRadioButton::toggled, _ui.time, &QLineEdit::setEnabled);
                connect(_ui.allRadio, &QRadioButton::toggled, this, [this](bool checked)
                {
                    Q_UNUSED(checked);
                    _ui.distance->setEnabled(false);
                    _ui.time->setEnabled(false);
                }
                );
            }

            void DeviceSettingsWidget::setDevice(mcc::ui::core::FlyingDevice* device)
            {
                _device = device;
                setWindowTitle(QString("Device Settings: %1").arg(_device->name()));

                auto pixmaps = device->manager()->pixmapList();

                for (auto p : pixmaps)
                {
                    auto item = new QListWidgetItem(QIcon(p), QString());
                    _ui.iconsList->addItem(item);
                }

                _ui.iconsList->setCurrentRow(0);
            }

            QPixmap DeviceSettingsWidget::pixmap() const
            {
                return _ui.iconsList->selectedItems().first()->icon().pixmap(32, 32);
            }

            int DeviceSettingsWidget::trailCount() const
            {
                if (_ui.distanceRadio->isChecked())
                    return _ui.distance->text().toInt();
                else if(_ui.timeRadio->isChecked())
                    return _ui.time->text().toInt();

                return 0;
            }

            mcc::ui::core::TrailMode DeviceSettingsWidget::trailMode() const
            {
                if (_ui.distanceRadio->isChecked())
                    return mcc::ui::core::TrailMode::Distance;
                else if (_ui.timeRadio->isChecked())
                    return mcc::ui::core::TrailMode::Time;
                else
                    return mcc::ui::core::TrailMode::All;
            }

            bool DeviceSettingsWidget::clearTrail() const
            {
                return _ui.clearTail->isChecked();
            }

        }
    }
}
