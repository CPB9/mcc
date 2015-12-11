/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "DeviceStateWidget.h"
#include "mcc/ui/flightcontrols/DeviceSettingsWidget.h"
#include "mcc/ui/flightcontrols/SliderCheckBox.h"

#include <QToolButton>

DeviceStateWidget::DeviceStateWidget(QWidget *parent)
    : QWidget(parent)
    , _currentState(Unknown)
    , _device(nullptr)
{
    ui.setupUi(this);

    _palette = palette();
    QColor backgroundColor = palette().color(QPalette::Base);

    _badAnimation = new QPropertyAnimation(this, "animationBrush");
    _badAnimation->setDuration(3000);
    _badAnimation->setKeyValueAt(0, backgroundColor);
    _badAnimation->setKeyValueAt(0.2, QColor(Qt::red));
    _badAnimation->setKeyValueAt(0.4, QColor(Qt::white));
    _badAnimation->setKeyValueAt(0.6, QColor(Qt::red));
    _badAnimation->setKeyValueAt(0.8, QColor(Qt::white));
    _badAnimation->setEndValue(backgroundColor);

    _goodAnimation = new QPropertyAnimation(this, "animationBrush");
    _goodAnimation->setDuration(3000);
    _goodAnimation->setKeyValueAt(0, backgroundColor);
    _goodAnimation->setKeyValueAt(0.2, QColor(Qt::green));
    _goodAnimation->setKeyValueAt(0.4, QColor(Qt::white));
    _goodAnimation->setKeyValueAt(0.6, QColor(Qt::green));
    _goodAnimation->setKeyValueAt(0.8, QColor(Qt::white));
    _goodAnimation->setEndValue(backgroundColor);

    connect(_badAnimation, &QAbstractAnimation::finished, this, &DeviceStateWidget::animationFinished);
    connect(_goodAnimation, &QAbstractAnimation::finished, this, &DeviceStateWidget::animationFinished);


    connect(ui.activateDeviceSlider, &mcc::ui::ide::SliderCheckBox::sliderStateChanged, this, [&](bool state)
    {
        emit requestActivateDevice(_device->name(), state);
    }
    );

    connect(ui.btnClearRoute, &QPushButton::pressed, this,
        [this]()
        {
            if (_device)
                _device->clearTail();
        }
    );
}

DeviceStateWidget::~DeviceStateWidget()
{
    delete _goodAnimation;
    delete _badAnimation;
}

void DeviceStateWidget::setDeviceName(const QString& name)
{
    ui.deviceName->setText(name);
}

void DeviceStateWidget::setPixmap(const QPixmap& pixmap)
{
    ui.icon->setPixmap(pixmap);
}

void DeviceStateWidget::setState(const QString& state1, const QString& state2)
{
    ui.state1->setText(state1);
    ui.state2->setText(state2);
}

void DeviceStateWidget::setModel(mcc::ui::core::FlyingDevice* model)
{
    _device = model;

    connect(_device, &mcc::ui::core::FlyingDevice::stateChanged, this, &DeviceStateWidget::setState);
    connect(_device, &mcc::ui::core::FlyingDevice::deviceStateChanged, this, &DeviceStateWidget::setDeviceState);
    connect(_device, &mcc::ui::core::FlyingDevice::pixmapChanged, this, &DeviceStateWidget::setPixmap);
    connect(_device, &mcc::ui::core::FlyingDevice::deviceDescriptionUpdated, this, &DeviceStateWidget::setDeviceDescription);
}

QColor DeviceStateWidget::animationBrush() const
{
    return palette().background().color();
}

void DeviceStateWidget::setAnimationBrush(const QColor& color)
{
    auto p = palette();
    p.setColor(QPalette::Base, color);
    setPalette(p);
//    update();
}

void DeviceStateWidget::setEnabled(bool flag)
{
    if (!flag) {
//        ui.signal->setValue(0);
    }
    ui.icon->setEnabled(flag);

    if (flag)
    {
        if (_badAnimation->state() == QAbstractAnimation::State::Running)
            _badAnimation->stop();

        if (_goodAnimation->state() == QAbstractAnimation::State::Running)
            return;

        setAutoFillBackground(true);

        _goodAnimation->start();
    }
    else
    {
        if (_goodAnimation->state() == QAbstractAnimation::State::Running)
            _goodAnimation->stop();

        if (_badAnimation->state() == QAbstractAnimation::State::Running)
            return;

        setAutoFillBackground(true);
        _badAnimation->start();
    }
}

void DeviceStateWidget::animationFinished()
{
    setAutoFillBackground(false);
    setPalette(_palette);
//    ui.signal->restoreCanvasBorder();
    update();
}

void DeviceStateWidget::setConnectionState(DeviceState state, const QString& reason)
{
    switch (state)
    {
    case Registration:
        if (_currentState != state)
            ui.connectionState->setStyleSheet("color:green");

        ui.connectionState->setText(QString("Регистрация: %1%").arg(reason));
        break;
    case Connected:
        if (_currentState != state)
            ui.connectionState->setStyleSheet("color:green");
        ui.connectionState->setText(QString("Подключено"));
        break;
    case Disconnected:
        if (_currentState != state)
            ui.connectionState->setStyleSheet("color:red");

        ui.connectionState->setText("Отключено");
        break;
    case Unknown:
        Q_ASSERT(false);
    }

    _currentState = state;
}

void DeviceStateWidget::setDeviceDescription(const mcc::misc::DeviceDescription& desc)
{
    ui.deviceName->setText(desc._device_info);
    ui.firmwareName->setText(QString("%1 (%2)").arg(desc._firmware_info).arg(desc._firmware_name));
}

void DeviceStateWidget::setDeviceState()
{
    mcc::misc::DeviceState state = _device->deviceState();

    ui.activateDeviceSlider->setChecked(state._isActive);

    if (!state._isActive)
    {
        setConnectionState(Disconnected);
    }
    else if (state._isActive && !state._isRegistered)
    {
        setConnectionState(Registration, QString::number(state._regState));
    }
    else
    {
        setConnectionState(Connected);
    }

    ui.netStats->updateStats(std::move(state._stats));
}
