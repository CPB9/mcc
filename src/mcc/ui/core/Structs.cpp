/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "Structs.h"

#include <cstdint>
#include <cassert>
#include <cstdlib>

namespace mcc {
    namespace ui {
        namespace core {

            bool doubleEq(double a, double b, unsigned int maxUlps)
            {
                // http://www.cygnus-software.com/papers/comparingfloats/comparingfloats.htm
                assert(maxUlps < 4 * 1024 * 1024);
                static_assert(std::numeric_limits<double>::is_iec559, "Iec559 double required");
                int64_t aInt = *(int64_t*)&a;
                if (aInt < 0) {
                    aInt = 0x8000000000000000ll - aInt;
                }
                int64_t bInt = *(int64_t*)&b;
                if (bInt < 0) {
                    bInt = 0x8000000000000000ll - bInt;
                }
                int64_t intDiff = std::abs(aInt - bInt);
                if (intDiff <= maxUlps) {
                    return true;
                }
                return false;
            }


            bool operator ==(const GeoPosition& left, const GeoPosition& right)
            {
                return doubleEq(left.latitude, right.latitude)
                    && doubleEq(left.longitude, right.longitude)
                    && doubleEq(left.altitude, right.altitude);
            }

            bool operator !=(const GeoPosition& left, const GeoPosition& right)
            {
                return !(left == right);
            }

            bool operator ==(const Waypoint& left, const Waypoint& right)
            {
                return left.index    == right.index
                    && left.position == right.position
                    && doubleEq(left.speed, right.speed)
                    && left.flags    == right.flags;
            }

            bool operator !=(const Waypoint& left, const Waypoint& right)
            {
                return !(left == right);
            }

            bool operator==(const GeoOrientation& left, const GeoOrientation& right)
            {
                return doubleEq(left.heading, right.heading)
                    && doubleEq(left.pitch, right.pitch)
                    && doubleEq(left.roll, right.roll);
            }


            bool operator!=(const GeoOrientation& left, const GeoOrientation& right)
            {
                return !(left == right);
            }

        }
    }
}

