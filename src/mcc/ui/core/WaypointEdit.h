/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#pragma once

#include "mcc/ui/core/Structs.h"

#include <QWidget>
#include <QString>

class QDoubleSpinBox;

namespace mcc {
namespace ui {
namespace core {

class LatLonEdit;
class WaypointFlagEdit;

class WaypointEdit : public QWidget {
    Q_OBJECT
public:
    explicit WaypointEdit(QWidget* parent = 0);
    explicit WaypointEdit(const Waypoint& waypoint, QWidget* parent = 0);

    void setWaypoint(const Waypoint& waypoint);
    Waypoint waypoint() const;

signals:
    void waypointChanged(const Waypoint& waypoint);

private:
    LatLonEdit* _latLonEdit;
    QDoubleSpinBox* _heightEdit;
    QDoubleSpinBox* _speedEdit;
    WaypointFlagEdit* _flagEdit;
};
}
}
}
