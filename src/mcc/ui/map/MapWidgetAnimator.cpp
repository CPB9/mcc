/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "mcc/ui/map/MapWidgetAnimator.h"
#include "mcc/ui/map/MapWidget.h"

#include <QTimer>
#include <QCursor>
#include <QDebug>

#include <cmath>

namespace mcc {
namespace ui {
namespace map {

MapWidgetAnimator::MapWidgetAnimator(MapWidget* parent)
    : QObject(parent)
    , _parent(parent)
    , _registerPosTimer(new QTimer(this))
    , _animateTimer(new QTimer(this))
{
    connect(_registerPosTimer.get(), &QTimer::timeout, this, &MapWidgetAnimator::setPosition);
    auto period = std::chrono::high_resolution_clock::period();
    double ratio = double(period.num) / period.den;
    _minSpeed = 5 * ratio;
    _decrement = ratio * 2;

    connect(_animateTimer.get(), &QTimer::timeout, this, [this]() {
        auto time = std::chrono::high_resolution_clock::now();
        auto timeDelta = time - _startTime;
        _startTime = time;
        QPointF delta = _speed * timeDelta.count();
        _accumulator += delta;
        QPoint intDelta = _accumulator.toPoint();
        if (!intDelta.isNull()) {
            _parent->scroll(intDelta);
            _accumulator -= intDelta;
        }
        double speedValue = std::hypot(_speed.x(), _speed.y());
        if (speedValue < _minSpeed) {
            stop();
        }
        _speed *= (1 - _decrement * timeDelta.count());
    });
}

void MapWidgetAnimator::setPosition()
{
    QPoint pos = QCursor::pos();
    QPointF posDelta = pos - _pos;
    _pos = pos;
    auto time = std::chrono::high_resolution_clock::now();
    auto timeDelta = time - _startTime;
    _startTime = time;
    _startSpeed = posDelta / timeDelta.count();
}

void MapWidgetAnimator::start()
{
    _speed = _startSpeed;
    _accumulator = QPointF(0, 0);
    _registerPosTimer->stop();
    _animateTimer->start(16);
}

void MapWidgetAnimator::stop()
{
    _animateTimer->stop();
    _startSpeed = QPointF(0, 0);
    _pos = QCursor::pos();
    _registerPosTimer->start(10);
}
}
}
}
