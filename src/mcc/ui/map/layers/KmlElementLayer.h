/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#pragma once

#include "mcc/ui/map/layers/SimpleFlagLayer.h"
#include "mcc/misc/Option.h"

#include "kml/dom/kml_ptr.h"
#include "kml/dom/style.h"
#include "kml/dom/geometry.h"

namespace mcc {
namespace ui {
namespace map {

class KmlElementLayer : public SimpleFlagLayer {
    Q_OBJECT
public:
    enum Type { Point, LineString, Polygon };

    KmlElementLayer(const kmldom::CoordinatesPtr& coordinates, const kmldom::StylePtr& style,
                    const MapRectConstPtr& rect);

    bool isA(Type type) const;

    virtual bool canAddPoints() const = 0;
    virtual Type type() const = 0;
    virtual void setActive(bool isActive) = 0;
    virtual bool hasElementAt(const QPointF& pos) const = 0;

protected:
    // const kmldom::CoordinatesPtr& kmlCoordinates() const;
    // const kmldom::StylePtr& kmlStyle() const;
    virtual void updateKml() = 0;

    kmldom::CoordinatesPtr _coordinates;
    kmldom::StylePtr _style;
};

inline bool KmlElementLayer::isA(KmlElementLayer::Type type) const
{
    return type == this->type();
}

inline KmlElementLayer::KmlElementLayer(const kmldom::CoordinatesPtr& coordinates, const kmldom::StylePtr& style,
                                        const MapRectConstPtr& rect)
    : SimpleFlagLayer(rect)
    , _coordinates(coordinates)
    , _style(style)
{
}

// inline void KmlElementLayer::setKmlCoordinates(const kmldom::CoordinatesPtr& coordinates)
// {
//     _coordinates = coordinates;
//     updateKml();
// }
//
// inline void KmlElementLayer::setKmlStyle(const kmldom::StylePtr& style)
// {
//     _style = style;
//     updateKml();
// }
}
}
}
