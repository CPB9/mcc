/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include <QtGlobal>
#include "bmcl/MemWriter.h"
#include "mcc/misc/Crc.h"
#include "mcc/encoder/photon/Photon.h"

namespace mcc {
namespace encoder {
namespace photon {

    bmcl::Result<std::vector<uint8_t>, ExchangeError> PhotonCoder::encodePacket(const uint8_t* request)
    {
        std::vector<uint8_t> packet;
        return packet;
    }

    template<int s, int t> struct check_size {
        static_assert(s == t, "wrong size");
    };

    bmcl::Result<PacketDetails, ExchangeError> PhotonCoder::decodePacket(const void* start, std::size_t size)
    {
        return ExchangeError::DecodeNotImplemented;
    }


}
}
}
