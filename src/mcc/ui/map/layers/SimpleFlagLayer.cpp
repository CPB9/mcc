/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "mcc/ui/map/layers/SimpleFlagLayer.h"
#include <MapRect.h>

#include <QPoint>
#include <QPointF>
#include <QSize>
#include <QApplication>

namespace mcc {
namespace ui {
namespace map {

SimpleFlagLayer::SimpleFlagLayer(const MapRectConstPtr& rect)
    : Layer(rect)
    , _activeFlag(misc::None)
    , _activeLine(misc::None)
    , _isMoving(false)
    , _hasMoved(false)
{
}

bool SimpleFlagLayer::mousePressEvent(const QPoint& pos)
{
    if (_activeFlag.isSome()) {
        _isMoving = true;
        _hasMoved = false;
        return true;
    } else if (_activeLine.isSome()) {
        insertFlagAt(_activeLine.unwrap().index + 1, _activeLine.unwrap().pos);
        _activeFlag = _activeLine.unwrap().index + 1;
        _activeLine = misc::None;
        setCurrentActive(true);
        _isMoving = true;
        _hasMoved = true;
        return true;
    } else if (QApplication::keyboardModifiers() & Qt::ControlModifier) {
        _isMoving = true;
        _hasMoved = true;
        addFlagAt(pos);
        return true;
    }
    _isMoving = false;
    _hasMoved = false;
    return false;
}

bool SimpleFlagLayer::mouseMoveEvent(const QPoint& oldPos, const QPoint& newPos)
{
    if (_activeFlag.isSome() && _isMoving) {
        QPoint delta = newPos - oldPos;
        _hasMoved = true;
        moveCurrentBy(delta);
        return true;
    } else {
        if (_activeFlag.isSome()) {
            setCurrentActive(false);
        }
        _activeFlag = flagAt(newPos);
        if (_activeFlag.isSome()) {
            _activeLine = misc::None;
            setCurrentActive(true);
        } else {
            _activeLine = lineAt(newPos);
        }
    }
    return false;
}

bool SimpleFlagLayer::viewportResizeEvent(const QSize& oldSize, const QSize& newSize)
{
    Q_UNUSED(oldSize);
    Q_UNUSED(newSize);
    return false;
}

bool SimpleFlagLayer::viewportScrollEvent(const QPoint& oldPos, const QPoint& newPos)
{
    (void)oldPos;
    (void)newPos;
    return true;
}

bool SimpleFlagLayer::contextMenuEvent(const QPoint& pos)
{
    QPoint pos2 = pos;
    pos2.setX(pos.x() % mapRect()->maxMapSize());
    _activeFlag = flagAt(pos2);
    if (_activeFlag.isSome()) {
        return showContextMenuForCurrent(pos);
    } else {
        return showContextMenuForNone(pos);
    }
}

bool SimpleFlagLayer::mouseReleaseEvent(const QPoint& pos)
{
    Q_UNUSED(pos);
    bool accepted = false;
    if (_activeFlag.isSome()) {
        if (_hasMoved) {
            finishMovingCurrent();
        } else {
            showFlagEditor(pos);
            accepted = true;
        }
    }
    _isMoving = false;
    _hasMoved = false;
    return accepted;
}
}
}
}
