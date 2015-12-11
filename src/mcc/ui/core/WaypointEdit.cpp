/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "mcc/ui/core/WaypointEdit.h"
#include "mcc/ui/core/WaypointFlagEdit.h"
#include "mcc/ui/core/LatLonEdit.h"

#include <QGridLayout>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QDoubleSpinBox>
#include <QLabel>

namespace mcc {
namespace ui {
namespace core {

WaypointEdit::WaypointEdit(QWidget* parent)
    : QWidget(parent)
    , _latLonEdit(new LonEdit) //HACK: ждем сдвоенного редактора
    , _heightEdit(new QDoubleSpinBox)
    , _speedEdit(new QDoubleSpinBox)
    , _flagEdit(new WaypointFlagEdit)
{
    _heightEdit->setMinimum(-11000.0);
    _heightEdit->setMaximum(40000000.0);
    _speedEdit->setSingleStep(100);
    _heightEdit->setSuffix("м");

    _speedEdit->setMinimum(-299792460.0);
    _speedEdit->setMaximum(299792460.0);
    _speedEdit->setSingleStep(10);
    _speedEdit->setSuffix("м/c");

    QGridLayout* layout = new QGridLayout;
    layout->addWidget(new QLabel("Координаты:"), 0, 0);
    layout->addWidget(new QLabel("Высота:"), 1, 0);
    layout->addWidget(new QLabel("Скорость:"), 2, 0);
    layout->addWidget(new QLabel("Флаги:"), 3, 0);
    layout->addWidget(_latLonEdit, 0, 1);
    layout->addWidget(_heightEdit, 1, 1);
    layout->addWidget(_speedEdit, 2, 1);
    layout->addWidget(_flagEdit, 3, 1);

    QPushButton* okButton = new QPushButton("Ок");
    QPushButton* cancelButton = new QPushButton("Отменить");
    QPushButton* applyButton = new QPushButton("Применить");
    QHBoxLayout* buttonLayout = new QHBoxLayout;
    buttonLayout->addStretch();
    buttonLayout->addWidget(okButton);
    buttonLayout->addWidget(cancelButton);
    buttonLayout->addWidget(applyButton);

    QVBoxLayout* mainLayout = new QVBoxLayout;
    mainLayout->addLayout(layout);
    mainLayout->addLayout(buttonLayout);
    setLayout(mainLayout);

    connect(okButton, &QPushButton::clicked, this, [this](){
        emit waypointChanged(waypoint());
        close();
    });

    connect(cancelButton, &QPushButton::clicked, this, &WaypointEdit::close);

    connect(applyButton, &QPushButton::clicked, this, [this](){
        emit waypointChanged(waypoint());
    });
}

WaypointEdit::WaypointEdit(const Waypoint& waypoint, QWidget* parent)
    : WaypointEdit(parent)
{
    setWaypoint(waypoint);
}

void WaypointEdit::setWaypoint(const Waypoint& waypoint)
{
    LatLon latLon;
    latLon.latitude = waypoint.position.latitude;
    latLon.longitude = waypoint.position.longitude;
    _latLonEdit->setLatLon(latLon);
    _heightEdit->setValue(waypoint.position.altitude);
    _speedEdit->setValue(waypoint.speed);
    _flagEdit->setFlags(waypoint.flags);
}

Waypoint WaypointEdit::waypoint() const
{
    Waypoint waypoint;
    LatLon latLon = _latLonEdit->latLon();
    waypoint.position.latitude = latLon.latitude;
    waypoint.position.longitude = latLon.longitude;
    waypoint.position.altitude = _heightEdit->value();
    waypoint.speed = _speedEdit->value();
    waypoint.flags = _flagEdit->flags();
    return waypoint;
}
}
}
}
