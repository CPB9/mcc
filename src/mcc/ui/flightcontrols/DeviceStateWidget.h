/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef DEVICE_STATE_WIDGET_H
#define DEVICE_STATE_WIDGET_H

#include <QWidget>
#include <QPropertyAnimation>

#include "mcc/ui/flightcontrols/ui_DeviceStateWidget.h"

#include "mcc/ui/core/Structs.h"
#include "mcc/ui/core/FlyingDevice.h"

#include "mcc/ui/core/GlobalCoordinatePrinter.h"

#include "mcc/misc/Device.h"

class DeviceStateWidget : public QWidget
{
    Q_OBJECT

    Q_PROPERTY(QColor animationBrush READ animationBrush WRITE setAnimationBrush)

    enum DeviceState
    {
        Unknown,
        Registration,
        Connected,
        Disconnected
    };

signals:
    void requestActivateDevice(const QString& device, bool isActive);

public:
    explicit DeviceStateWidget(QWidget *parent = 0);
    ~DeviceStateWidget();

    QColor animationBrush() const;
    void setAnimationBrush(const QColor& color);

    void setEnabled(bool flag);
    void setConnectionState(DeviceState state, const QString& reason = QString());

    mcc::ui::core::FlyingDevice* device() const { return _device; }

public slots:
    void setDeviceName      (const QString& name);
    void setPixmap          (const QPixmap& pixmap);
    void setState            (const QString& state1, const QString& state2);
    void setDeviceState();

    void setModel(mcc::ui::core::FlyingDevice* model);

    void animationFinished();

    void setDeviceDescription(const mcc::misc::DeviceDescription& desc);
private:
    void setPosition(const mcc::ui::core::GeoPosition& pos);
private:
    Ui::DeviceStateWidget ui;

    DeviceState _currentState;

    mcc::ui::core::FlyingDevice* _device;

    QPropertyAnimation* _goodAnimation;
    QPropertyAnimation* _badAnimation;
    QPalette _palette;
    QString _mode;
};

#endif // DEVICE_STATE_WIDGET_H
