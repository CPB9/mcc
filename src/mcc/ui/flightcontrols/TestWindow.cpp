/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "TestWindow.h"
#include "PrimaryFlightDisplay.h"

#include <cmath>

TestWindow::TestWindow(QWidget *parent, PrimaryFlightDisplay* primaryFlightDisplay)
    : QWidget(parent), _primaryFlightDisplay(primaryFlightDisplay)
{
    _realTime = 0.0f;

    ui.setupUi(this);

    connect(ui.rollValue, SIGNAL(valueChanged(double)), _primaryFlightDisplay, SLOT(setRoll(double)));
    connect(ui.pitchValue, SIGNAL(valueChanged(double)), _primaryFlightDisplay, SLOT(setPitch(double)));
    connect(ui.airSpeedValue, SIGNAL(valueChanged(double)), _primaryFlightDisplay, SLOT(setAirSpeed(double)));
    connect(ui.altitudeValue, SIGNAL(valueChanged(double)), _primaryFlightDisplay, SLOT(setAltitude(double)));
    connect(ui.headingValue, SIGNAL(valueChanged(double)), _primaryFlightDisplay, SLOT(setHeading(double)));
    connect(ui.targetHeadingValue, SIGNAL(valueChanged(double)), _primaryFlightDisplay, SLOT(setTargetHeading(double)));


    startTimer(0);
    _time.start();
}

TestWindow::~TestWindow()
{

}

void TestWindow::timerEvent(QTimerEvent * event)
{
    QWidget::timerEvent(event);

    float timeStep = _time.restart();

    _realTime = _realTime + timeStep / 500.0f;

    float roll = 0.0f;
    float pitch = 0.0f;
    float airspeed = 0.0f;

    if (ui.pitchAuto->isChecked())
    {
        pitch = 30.0f * sin(_realTime / 10.0f);
        ui.pitchValue->setValue(pitch);
    }

    if (ui.rollAuto->isChecked())
    {
        roll = 50.0f * sin(_realTime / 5.0f);
        ui.rollValue->setValue(roll);
    }

    airspeed = 125.0f * sin(_realTime / 40.0f) + 125.0f;
    ui.airSpeedValue->setValue(airspeed);

    float altitude = 1000.0f * sin(_realTime / 40.0f) + 1000.0f;
    ui.altitudeValue->setValue(altitude);

    if (ui.headingAuto->isChecked())
    {
        float heading = 360.0f * sin(_realTime / 40.0f) - 180;
        ui.headingValue->setValue(heading);
    }

    if (ui.targetHeadingAuto->isChecked())
    {
        float heading = 360.0f * cos(_realTime / 40.0f);
        ui.targetHeadingValue->setValue(heading);
    }
}
