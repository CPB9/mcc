/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "mcc/modeling/2DMath.h"

namespace mcc
{
namespace modeling
{

const double metersInDegree = 111319.9;


double normalizeAngle180Deg(double angleDeg)
{
    angleDeg = std::fmod(angleDeg, 360.);
    return angleDeg > 180. ? angleDeg - 360. : (angleDeg < -180. ? angleDeg + 360. : angleDeg);
}

double normalizePitchDeg(double pitchDeg)
{
    double result = normalizeAngle180Deg(pitchDeg);
    BMCL_ASSERT(result >= -90. && result <= 90.);
    return result;
}

void advanceLatitudeLongitudeDeg(double &latDeg, double &longDeg, double angleDeg, double speedMs)
{
    double latDeltaM(std::cos(misc::degreesToRadians(angleDeg)) * speedMs), longDeltaM(std::sin(misc::degreesToRadians(angleDeg)) * speedMs);
    double latDeltaDeg(mToDeg(latDeltaM)), longDeltaDeg(mToDeg(longDeltaM));
    latDeg += latDeltaDeg;
    longDeg += longDeltaDeg;
}

double distance(double x1, double y1, double x2, double y2)
{
    return std::sqrt(std::pow(x2 - x1, 2.) + std::pow(y2 - y1, 2.));
}

double distanceToLine(double lineX1, double lineY1, double lineX2, double lineY2, double x, double y)
{
    return std::abs((lineY2 - lineY1) * x - (lineX2 - lineX1) * y + lineX2 * lineY1 - lineY2 * lineX1) / std::sqrt(std::pow(lineY2 - lineY1, 2.0) + std::pow(lineX2 - lineX1, 2.0));
}

double distanceToLineSegment(double x1, double y1, double x2, double y2, double pointX, double pointY)
{
    double diffX = x2 - x1;
    double diffY = y2 - y1;
    if ((diffX == 0) && (diffY == 0))
    {
        diffX = pointX - x1;
        diffY = pointY - y1;
        return sqrt(diffX * diffX + diffY * diffY);
    }

    double t = ((pointX - x1) * diffX + (pointY - y1) * diffY) / (diffX * diffX + diffY * diffY);

    if (t < 0)
    {
        //point is nearest to the first point i.e x1 and y1
        diffX = pointX - x1;
        diffY = pointY - y1;
    }
    else if (t > 1)
    {
        //point is nearest to the end point i.e x2 and y2
        diffX = pointX - x2;
        diffY = pointY - y2;
    }
    else
    {
        //if perpendicular line intersect the line segment.
        diffX = pointX - (x1 + t * diffX);
        diffY = pointY - (y1 + t * diffY);
    }

    //returning shortest distance
    return sqrt(diffX * diffX + diffY * diffY);
}


double normalizeLatitudeDeg(double latitudeDeg)
{
    if (latitudeDeg > 85.) {
        return 85.;
    }
    if (latitudeDeg < -85.) {
        return -85.;
    }
    return latitudeDeg;
}

double angleFromCoordsRad(double lat1Rad, double long1Rad, double lat2Rad, double long2Rad)
{
    double longDeltaRad(long2Rad - long1Rad);
    return std::atan2(std::sin(longDeltaRad) * std::cos(lat2Rad),
                      std::cos(lat1Rad) * std::sin(lat2Rad) - std::sin(lat1Rad) * std::cos(lat2Rad) * std::cos(longDeltaRad));
}

double angleFromCoordsDeg(double lat1Deg, double long1Deg, double lat2Deg, double long2Deg)
{
    return misc::radiansToDegrees(angleFromCoordsRad(misc::degreesToRadians(lat1Deg), misc::degreesToRadians(long1Deg),
                                                     misc::degreesToRadians(lat2Deg), misc::degreesToRadians(long2Deg)));
}

}
}