/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#pragma once

#include "mcc/ui/map/Ptr.h"
#include "bmcl/Option.h"

#include <QtMath>

#include <cmath>
#include <utility>

namespace mcc {
namespace ui {
namespace map {

template <typename B>
class EmptyWrapper : public B {
public:
    template <typename... A>
	EmptyWrapper(bmcl::InPlaceType, A&&... args)
        : B(std::forward<A>(args)...)
    {
    }

    void reset(B&& base)
    {
        B::operator=(std::forward<B>(base));
    }

    void update(const EmptyWrapper<B>& next, const MapRectConstPtr& rect)
    {
        (void)next;
        (void)rect;
    }

    void setSingle()
    {
    }
};

template <typename B>
class AngleWrapper : public B {
public:
    template <typename... A>
    AngleWrapper(bmcl::InPlaceType, A&&... args)
		: B(bmcl::InPlace, std::forward<A>(args)...)
        , _angle(90)
    {
    }

    void setAngle(double angle)
    {
        _angle = angle;
    }

    double angle() const
    {
        return _angle;
    }

    void update(const AngleWrapper<B>& next, const MapRectConstPtr& rect)
    {
        B::update(next, rect);
        const QPointF& p1 = B::position();
        const QPointF& p2 = next.position();
        _angle = qRadiansToDegrees(std::atan2(-p2.y() + p1.y(), p2.x() - p1.x()));
    }

    void setSingle()
    {
        B::setSingle();
        _angle = 0;
    }

private:
    double _angle;
};

template <typename B>
class DistanceWrapper : public B {
public:
    template <typename... A>
    DistanceWrapper(bmcl::InPlaceType, A&&... args)
		: B(bmcl::InPlace, std::forward<A>(args)...)
        , _distance(0)
    {
    }

    void setDistance(double distance)
    {
        _distance = distance;
    }

    double distance() const
    {
        return _distance;
    }

    void update(const DistanceWrapper<B>& next, const MapRectConstPtr& rect)
    {
        B::update(next, rect);
        core::LatLon curLatLon = B::toLatLon(rect);
        core::LatLon nextLatLon = next.toLatLon(rect);
        _distance = rect->projection().calcDistance(curLatLon, nextLatLon);
    }

    void setSingle()
    {
        B::setSingle();
        _distance = 0;
    }

private:
    double _distance;
};
}
}
}
