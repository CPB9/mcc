/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#pragma once

#include "bmcl/Assert.h"

#include "mcc/misc/CommonMath.h"

namespace mcc
{
namespace modeling
{

class Point2D
{
public:
    Point2D(double x, double y) : _x(x), _y(y) {}
    double x() const { return _x; }
    double y() const { return _y; }
private:
    double _x, _y;
};

extern const double metersInDegree;

double normalizeAngle180Deg(double angleDeg);

inline double normalizeHeadingDeg(double headingDeg)
{
    return normalizeAngle180Deg(headingDeg);
}

inline double normalizeRollDeg(double rollDeg)
{
    return normalizeAngle180Deg(rollDeg);
}

double normalizePitchDeg(double pitchDeg);

inline double mToDeg(double meters)
{
    return meters / metersInDegree;
}

inline double degToM(double degs)
{
    return degs * metersInDegree;
}

void advanceLatitudeLongitudeDeg(double &latDeg, double &longDeg, double angleDeg, double speedMs);

double distance(double x1, double y1, double x2, double y2);

double distanceToLine(double lineX1, double lineY1, double lineX2, double lineY2, double x, double y);

double distanceToLineSegment(double x1, double y1, double x2, double y2, double pointX, double pointY);

inline double normalizeLongitudeDeg(double longitudeDeg)
{
    return fmod(longitudeDeg + 360., 360.);
}

double normalizeLatitudeDeg(double latitudeDeg);

inline double anglesDeltaDeg(double fromDeg, double toDeg)
{
    return normalizeAngle180Deg(toDeg - fromDeg);
}

double angleFromCoordsRad(double lat1Rad, double long1Rad, double lat2Rad, double long2Rad);

inline double angleFromCoordsRad(const Point2D& fromRad, const Point2D& toRad)
{
    return angleFromCoordsRad(fromRad.x(), fromRad.y(), toRad.x(), toRad.y());
}

double angleFromCoordsDeg(double lat1Deg, double long1Deg, double lat2Deg, double long2Deg);

}
}

