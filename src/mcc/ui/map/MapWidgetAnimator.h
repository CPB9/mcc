/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#pragma once

#include <QObject>
#include <QPoint>
#include <QPointF>

#include <memory>
#include <chrono>

class QTimer;

namespace mcc {
namespace ui {
namespace map {

class MapWidget;

class MapWidgetAnimator : public QObject {
public:
    MapWidgetAnimator(MapWidget* parent = 0);

    void start();
    void stop();

private slots:
    void setPosition();

private:
    MapWidget* _parent;
    std::unique_ptr<QTimer> _registerPosTimer;
    std::unique_ptr<QTimer> _animateTimer;
    QPointF _pos;
    QPointF _accumulator;
    QPointF _startSpeed;
    QPointF _speed;
    QPointF _speedDecrement;
    double _minSpeed;
    double _decrement;
    std::chrono::high_resolution_clock::time_point _startTime;
};
}
}
}
