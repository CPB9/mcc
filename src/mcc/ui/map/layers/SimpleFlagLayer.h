/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#pragma once

#include "mcc/ui/map/layers/Layer.h"
#include "mcc/ui/map/LineAndPos.h"
#include "mcc/misc/Option.h"
#include <QPointF>

class QSize;
class QPoint;

namespace mcc {
namespace ui {
namespace map {

class SimpleFlagLayer : public Layer {
    Q_OBJECT
public:
    SimpleFlagLayer(const MapRectConstPtr& rect);
    bool mouseMoveEvent(const QPoint& oldPos, const QPoint& newPos) override;
    bool mousePressEvent(const QPoint& pos) override;
    bool mouseReleaseEvent(const QPoint& pos) override;
    bool viewportResizeEvent(const QSize& oldSize, const QSize& newSize) override;
    bool viewportScrollEvent(const QPoint& oldPos, const QPoint& newPos) override;
    bool contextMenuEvent(const QPoint& pos) override;

protected:
    virtual void addFlagAt(const QPointF& pos) = 0;
    virtual void insertFlagAt(std::size_t index, const QPointF& pos) = 0;
    virtual misc::Option<std::size_t> flagAt(const QPointF& pos) = 0;
    virtual misc::Option<LineIndexAndPos> lineAt(const QPointF& pos) = 0;
    virtual void moveCurrentBy(const QPointF& delta) = 0;
    virtual void setCurrentActive(bool isActive) = 0;
    virtual bool showContextMenuForCurrent(const QPoint& pos) = 0;
    virtual bool showContextMenuForNone(const QPoint& pos) = 0;
    virtual void showFlagEditor(const QPoint& pos) = 0;
    virtual void finishMovingCurrent() = 0;

    misc::Option<std::size_t> _activeFlag;
    misc::Option<LineIndexAndPos> _activeLine;

private:
    bool _isMoving;
    bool _hasMoved;
};
}
}
}
