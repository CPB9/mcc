/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef TESTWINDOW_H
#define TESTWINDOW_H

#include <QWidget>
#include <QTime>

#include "mcc/ui/flightcontrols/ui_TestWindow.h"

#include "mcc/ui/flightcontrols/PrimaryFlightDisplay.h"

using namespace mcc::ui::flightcontrols;

class TestWindow : public QWidget
{
    Q_OBJECT

public:
    TestWindow(QWidget *parent, PrimaryFlightDisplay* primaryFlightDisplay);
    ~TestWindow();

    void timerEvent(QTimerEvent * event);
private:
    Ui::TestWindow ui;

    float _realTime;
    QTime _time;
    PrimaryFlightDisplay* _primaryFlightDisplay;
};

#endif // TESTWINDOW_H
