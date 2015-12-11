/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#pragma once

namespace mcc
{
namespace misc
{
class PointCoordinates
{
public:
    PointCoordinates(double latitudeDeg, double longitudeDeg, double altitudeM)
            : _latitudeDeg(latitudeDeg), _longitudeDeg(longitudeDeg), _altitudeM(altitudeM) {}
    double longitudeDeg() const { return _longitudeDeg; }
    void setLongitudeDeg(double longitudeDeg) { _longitudeDeg = longitudeDeg; }
    double latitudeDeg() const { return _latitudeDeg; }
    void setLatitudeDeg(double latitudeDeg) { _latitudeDeg = latitudeDeg; }
    double altitude() const { return _altitudeM; }
    void setAltitudeM(double altitudeM) { _altitudeM = altitudeM; }
protected:
    double _latitudeDeg, _longitudeDeg, _altitudeM;
};

class Orientation
{
public:
    Orientation(double headingDeg, double pitchDeg, double rollDeg)
            : _headingDeg(headingDeg), _pitchDeg(pitchDeg), _rollDeg(rollDeg) {}
public:
    double headingDeg() const { return _headingDeg; }
protected:
    double _headingDeg, _pitchDeg, _rollDeg;
};
}
}
