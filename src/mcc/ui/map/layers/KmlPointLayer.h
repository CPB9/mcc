/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#pragma once

#include "mcc/ui/map/layers/KmlElementLayer.h"
#include "mcc/ui/map/drawables/Flag.h"

class QMenu;

namespace mcc {
namespace ui {
namespace map {

class KmlPointLayer : public KmlElementLayer {
    Q_OBJECT
public:
    KmlPointLayer(const QString& label, const kmldom::CoordinatesPtr& coordinates, const kmldom::StylePtr& style,
                  const MapRectConstPtr& rect);
    ~KmlPointLayer() override;

    bool canAddPoints() const override;
    Type type() const override;
    void setActive(bool isActive) override;
    bool hasElementAt(const QPointF& pos) const override;

    void draw(QPainter* p) const override;
    bool zoomEvent(const QPoint& pos, int fromZoom, int toZoom) override;
    void changeProjection(const MercatorProjection& from, const MercatorProjection& to) override;
    bool viewportResetEvent(int oldZoom, int newZoom, const QRect& oldViewpiort, const QRect& newViewport) override;

protected:
    void updateKml() override;
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
    QMenu* _contextMenu;
    Flag _flag;
    QString _label;
    bool _isActive;
};
}
}
}
