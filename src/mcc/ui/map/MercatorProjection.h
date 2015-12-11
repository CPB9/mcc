/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#pragma once

#include "mcc/ui/core/Structs.h"

#include <QtMath>

#include <cmath>
#include <memory>

namespace mcc {
namespace ui {
namespace map {

class MercatorProjection;

typedef std::shared_ptr<MercatorProjection> MercatorProjectionPtr;

class MercatorProjection {
public:
    enum ProjectionType { EllipticalMercator, SphericalMercator };

    MercatorProjection();
    MercatorProjection(double a, double b);
    MercatorProjection(ProjectionType type);

    static MercatorProjectionPtr create();
    static MercatorProjectionPtr create(double a, double b);
    static MercatorProjectionPtr create(ProjectionType type);

    double calcDistance(double lat1, double lon1, double lat2, double lon2) const;
    double calcDistance(const core::LatLon& from, const core::LatLon& to) const;
    double longitudeToX(double lon) const;
    double longitudeToRelativeOffset(double lon) const;
    double xToLongitude(double x) const;
    double relativeOffsetToLatitude(double relativeOffset) const;
    double relativeOffsetToLongitude(double relativeOffset) const;
    double latitudeToRelativeOffset(double lat) const;
    double latitudeToY(double lat) const;
    double yToLatitude(double y) const;
    double scalingFactorFromLatitude(double lat) const;
    double scalingFactorFromY(double y) const;
    double mapWidth() const;
    double maxLatitude() const;
    double majorAxis() const;
    double minorAxis() const;
    double equatorCircumference() const;
    double parallelCircumference(double lat) const;

    bool isA(ProjectionType type) const;

private:
    double tsToLatitude(double ts) const;
    static double ellipseCircumference(double a, double b);
    void init(ProjectionType type);
    void init(double a, double b);

    double _a; // большая полуось
    double _b; // малая полуось
    double _e; // эксцентриситет
    double _maxLatitude;
    double _equatorCircumference;
};

inline MercatorProjectionPtr MercatorProjection::create()
{
    return std::make_shared<MercatorProjection>();
}

inline MercatorProjectionPtr MercatorProjection::create(double a, double b)
{
    return std::make_shared<MercatorProjection>(a, b);
}

inline MercatorProjectionPtr MercatorProjection::create(MercatorProjection::ProjectionType type)
{
    return std::make_shared<MercatorProjection>(type);
}

inline MercatorProjection::MercatorProjection()
{
    init(SphericalMercator);
}

inline MercatorProjection::MercatorProjection(double a, double b)
{
    init(a, b);
}

inline MercatorProjection::MercatorProjection(MercatorProjection::ProjectionType type)
{
    init(type);
}

inline double MercatorProjection::longitudeToX(double lon) const
{
    return _a * qDegreesToRadians(lon);
}

inline double MercatorProjection::longitudeToRelativeOffset(double lon) const
{
    return qDegreesToRadians(lon) / M_PI;
}

inline double MercatorProjection::xToLongitude(double x) const
{
    return qRadiansToDegrees(x / _a);
}

inline double MercatorProjection::relativeOffsetToLongitude(double relativeOffset) const
{
    return qRadiansToDegrees(relativeOffset * M_PI);
}

inline double MercatorProjection::mapWidth() const
{
    return _equatorCircumference;
}

inline double MercatorProjection::maxLatitude() const
{
    return qRadiansToDegrees(_maxLatitude);
}

inline double MercatorProjection::majorAxis() const
{
    return _a;
}

inline double MercatorProjection::minorAxis() const
{
    return _b;
}

inline double MercatorProjection::equatorCircumference() const
{
    return _equatorCircumference;
}

inline double MercatorProjection::parallelCircumference(double lat) const
{
    return _equatorCircumference * scalingFactorFromLatitude(lat);
}

inline double MercatorProjection::calcDistance(const core::LatLon& from, const core::LatLon& to) const
{
    return calcDistance(from.latitude, from.longitude, to.latitude, to.longitude);
}

inline double MercatorProjection::relativeOffsetToLatitude(double offset) const
{
    return tsToLatitude(std::exp(-offset / 0.5 * M_PI));
}

inline double MercatorProjection::yToLatitude(double y) const
{
    return tsToLatitude(std::exp(-y / _a));
}

inline double MercatorProjection::scalingFactorFromY(double y) const
{
    return std::cosh(2 * M_PI * y / _equatorCircumference);
}
}
}
}
