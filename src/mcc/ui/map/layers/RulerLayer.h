/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#pragma once

#include "mcc/ui/map/layers/SimpleFlagLayer.h"
#include "mcc/ui/map/drawables/RulerPolyLine.h"
#include "mcc/ui/map/drawables/Label.h"
#include "mcc/ui/map/drawables/BiMarker.h"

class QAction;
class QMenu;

namespace mcc {
namespace ui {
namespace map {

class RulerLayer : public SimpleFlagLayer {
    Q_OBJECT
public:
    RulerLayer(const MapRectConstPtr& rect);
    ~RulerLayer() override;
    void draw(QPainter* p) const override;
    bool zoomEvent(const QPoint& pos, int fromZoom, int toZoom) override;
    void changeProjection(const MercatorProjection& from, const MercatorProjection& to) override;
    bool viewportResetEvent(int oldZoom, int newZoom, const QRect& oldViewpiort, const QRect& newViewport) override;

    misc::Option<core::GeoBox> bbox() const;

protected:
    void addFlagAt(const QPointF& pos) override;
    void insertFlagAt(std::size_t index, const QPointF& pos) override;
    misc::Option<std::size_t> flagAt(const QPointF& pos) override;
    misc::Option<LineIndexAndPos> lineAt(const QPointF& pos) override;
    void moveCurrentBy(const QPointF& delta) override;
    void setCurrentActive(bool isActive) override;
    bool showContextMenuForCurrent(const QPoint& pos) override;
    bool showContextMenuForNone(const QPoint& pos) override;
    void showFlagEditor(const QPoint& pos) override;
    void finishMovingCurrent() override;

private:
    void removeActive();
    void execContextMenu(const QPoint& pos, bool hasActive);
    template <typename F, typename... A>
    void visitLabels(F func, A&&... args);
    RulerPolyLine<BiMarker> _points;
    QPixmap _activePixmap;
    QPixmap _inactivePixmap;
    QMenu* _contextMenu;
    QAction* _addAction;
    QAction* _clearAction;
    QAction* _removeAction;
    QAction* _copyAction;
};
}
}
}
