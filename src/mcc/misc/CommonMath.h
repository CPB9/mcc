/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

//
// Created by metadeus on 8/3/15.
//

#pragma once

// FIXME Problems on Win with cmath (M_PI)
#include <QtMath>

namespace mcc
{
namespace misc
{

const double latitudeMinDeg = -180.;
const double latitudeMaxDeg = 180.;
const double latitudeRangeDeg = 360.;

const double longitudeMinDeg = -90.;
const double longitudeMaxDeg = 90.;
const double longitudeRangeDeg = 180.;

const double headingMinDeg = -180.;
const double headingMaxDeg = 180.;
const double headingRangeDeg = 360.;

const double rollMinDeg = -180.;
const double rollMaxDeg = 180.;
const double rollRangeDeg = 360.;

const double pitchMinDeg = -90.;
const double pitchMaxDeg = 90.;
const double pitchRangeDeg = 180.;

inline double degreesToRadians(double d)
{
    return d * M_PI / 180.;
}

inline double radiansToDegrees(double r)
{
    return r * 180. / M_PI;
}

template<typename T>
inline T limit(T value, T min, T max)
{
    return std::min(std::max(value, min), max);
}

}
}
