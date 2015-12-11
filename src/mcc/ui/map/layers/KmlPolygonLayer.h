/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#pragma once

#include "mcc/ui/map/layers/KmlPolyLineLayer.h"
#include "mcc/ui/map/drawables/Marker.h"
#include "mcc/ui/map/drawables/PolyLine.h"

namespace mcc {
namespace ui {
namespace map {

class KmlPolygonLayer : public KmlPolyLineLayer {
    Q_OBJECT
public:
    KmlPolygonLayer(const QString& label, const kmldom::CoordinatesPtr& coordinates, const kmldom::StylePtr& style,
                    const MapRectConstPtr& rect);
    ~KmlPolygonLayer() override;

    Type type() const override;
    void draw(QPainter* p) const override;
    bool hasElementAt(const QPointF& pos) const override;

    void addFlagAt(const QPointF& pos) override;
    void insertFlagAt(std::size_t index, const QPointF& pos) override;
    misc::Option<std::size_t> flagAt(const QPointF& pos) override;
    void moveCurrentBy(const QPointF& delta) override;

protected:
    void removeActive() override;
};
}
}
}
