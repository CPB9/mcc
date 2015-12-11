/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "mcc/ui/map/MercatorProjection.h"

#include <QtMath>

#include <cassert>
#include <cmath>
#include <cstdlib>
#include <cstdint>

namespace mcc {
namespace ui {
namespace map {

double MercatorProjection::calcDistance(double lat1, double lon1, double lat2, double lon2) const
{
    double slat = std::sin((qDegreesToRadians(lat2 - lat1)) / 2);
    double slon = std::sin(qDegreesToRadians((lon2 - lon1) / 2));
    double a
        = std::fma(slon * slon, std::cos(qDegreesToRadians(lat1)) * std::cos(qDegreesToRadians(lat2)), slat * slat);
    double c = 2 * std::atan2(std::sqrt(a), std::sqrt(1 - a));
    return _a * c;
}

double MercatorProjection::latitudeToRelativeOffset(double lat) const
{
    lat = qDegreesToRadians(lat);
    double esinlat = _e * std::sin(lat);
    return 0.5 * std::log(std::tan(std::fma(0.5, lat, M_PI_4)) * std::pow((1 - esinlat) / (1 + esinlat), _e / 2))
        / M_PI;
}

double MercatorProjection::latitudeToY(double lat) const
{
    lat = qDegreesToRadians(lat);
    double esinlat = _e * std::sin(lat);
    return _a * std::log(std::tan(std::fma(0.5, lat, M_PI_4)) * std::pow((1 - esinlat) / (1 + esinlat), _e / 2));
}

// c wiki OSM

double MercatorProjection::tsToLatitude(double ts) const
{
    double lat = std::fma(-2, std::atan(ts), M_PI_2);
    double dlat = 1.0;
    double prec = 0.000000001;
    int i = 0;
    while ((std::abs(dlat) > prec) && (i < 15)) {
        double esinlat = _e * std::sin(lat);
        dlat = std::fma(-2, std::atan(ts * std::pow((1 - esinlat) / (1 + esinlat), _e / 2)), M_PI_2 - lat);
        lat += dlat;
        i++;
    }
    return qRadiansToDegrees(lat);
}

double MercatorProjection::scalingFactorFromLatitude(double lat) const
{
    lat = qDegreesToRadians(lat);
    return std::sqrt(1 - std::pow(_e * std::sin(lat), 2)) / std::cos(lat);
}

double MercatorProjection::ellipseCircumference(double a, double b)
{
    // аппроксимация периметра эллипса по Бесселю
    double h = std::pow((a - b) / (a + b), 2);
    double c[15] = {16.0, 4.0, 2.56, 2.0408163265306123, 1.7777777777777777, 1.6198347107438016, 1.514792899408284,
                    1.44, 1.3840830449826989, 1.3407202216066483, 1.3061224489795917, 1.277882797731569, 1.2544,
                    1.2345679012345678, 1.2175980975029725};
    double hh = 0.25 * h;
    double s = 1;
    for (int i = 0; i < 15; i++) {
        s += hh;
        hh *= h / c[i];
    }
    return M_PI * (a + b) * s;
}

const double spherA = 6378137;
const double spherB = 6378137;
const double ellA = 6378137;
const double ellB = 6356752.314245;

void MercatorProjection::init(MercatorProjection::ProjectionType type)
{
    if (type == ProjectionType::EllipticalMercator) {
        init(ellA, ellB);
    } else {
        init(spherA, spherB);
    }
}

static bool doubleEq(double a, double b, unsigned int maxUlps = 4)
{
    // http://www.cygnus-software.com/papers/comparingfloats/comparingfloats.htm
    assert(maxUlps < 4 * 1024 * 1024);
    static_assert(std::numeric_limits<double>::is_iec559, "Iec559 double required");
    int64_t aInt = *(int64_t*)&a;
    if (aInt < 0) {
        aInt = 0x8000000000000000 - aInt;
    }
    int64_t bInt = *(int64_t*)&b;
    if (bInt < 0) {
        bInt = 0x8000000000000000 - bInt;
    }
    int64_t intDiff = std::abs(aInt - bInt);
    if (intDiff <= maxUlps) {
        return true;
    }
    return false;
}

bool MercatorProjection::isA(MercatorProjection::ProjectionType type) const
{
    if (type == SphericalMercator) {
        return doubleEq(spherA, _a) && doubleEq(spherB, _b);
    } else if (type == EllipticalMercator) {
        return doubleEq(ellA, _a) && doubleEq(ellB, _b);
    }
    return false;
}

void MercatorProjection::init(double a, double b)
{
    _a = std::max(a, b);
    _b = std::min(a, b);
    _e = std::sqrt(1 - std::pow(_b / _a, 2));
    _equatorCircumference = ellipseCircumference(_a, _b);
    _maxLatitude = qDegreesToRadians(yToLatitude(_equatorCircumference / 2));
}
}
}
}
