/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "mcc/ui/map/MapSlider.h"
#include "mcc/ui/map/WebMapProperties.h"

#include <QPushButton>
#include <QSlider>
#include <QVBoxLayout>
#include <QDebug>

namespace mcc {
namespace ui {
namespace map {

MapSlider::MapSlider(QWidget* parent)
    : QWidget(parent)
    , _isChangingValue(false)
{
    auto createButton = [this](const char* sign, int delta) {
        QPushButton* button = new QPushButton(sign);
        button->setMinimumHeight(20);
        button->setMinimumWidth(20);
        button->setMaximumWidth(20);
        button->setMaximumHeight(20);
        button->resize(20, 20);
        connect(button, &QPushButton::clicked, [this, delta]() {
            _slider->setValue(_slider->value() + delta);
            emit valueChanged(_slider->value());
        });
        return button;
    };
    _zoomInButton = createButton("+", 1);
    _zoomOutButton = createButton("-", -1);
    _slider = new QSlider;
    _slider->setMinimumHeight(200);
    _slider->setMinimum(WebMapProperties::minZoom());
    _slider->setMaximum(WebMapProperties::maxZoom());
    _slider->setValue(_slider->minimum());
    QVBoxLayout* layout = new QVBoxLayout;
    layout->addWidget(_zoomInButton);
    layout->addWidget(_slider);
    layout->addWidget(_zoomOutButton);
    layout->setContentsMargins(0, 0, 0, 0);
    setLayout(layout);
    connect(_slider, &QSlider::valueChanged, this, &MapSlider::onSliderValueChanged);
    //connect(_slider, &QSlider::sliderMoved, this, &MapSlider::valueChanged);
    onSliderValueChanged(_slider->value());
    adjustSize();
}

void MapSlider::setMaximum(int value)
{
    _slider->setMaximum(value);
    onSliderValueChanged(_slider->value());
}

void MapSlider::setMinimum(int value)
{
    _slider->setMinimum(value);
    onSliderValueChanged(_slider->value());
}

void MapSlider::setValue(int value)
{
    _isChangingValue = true;
    _slider->setValue(value);
    _isChangingValue = false;
}

int MapSlider::value() const
{
    return _slider->value();
}

void MapSlider::onSliderValueChanged(int value)
{
    _zoomInButton->setDisabled(value == _slider->maximum());
    _zoomOutButton->setDisabled(value == _slider->minimum());
    if (!_isChangingValue) {
        emit valueChanged(value);
    }
}
}
}
}
