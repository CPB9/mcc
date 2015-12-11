/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include <QtWidgets/QApplication>
#include "mcc/ui/flightcontrols/PrimaryFlightDisplay.h"
#include "mcc/ui/flightcontrols/TestWindow.h"

using namespace mcc::ui::flightcontrols;


int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    auto widget = new QWidget();
    auto layout = new QHBoxLayout();
    widget->setLayout(layout);

    PrimaryFlightDisplay* pfd = new PrimaryFlightDisplay(nullptr);
    pfd->init();

    layout->addWidget(pfd);
    layout->addWidget(new TestWindow(0, pfd));

    widget->show();
    widget->resize(800, 600);

    return a.exec();
}
