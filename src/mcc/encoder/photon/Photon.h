/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#pragma once
#include <vector>
#include <string>
#include "bmcl/Result.h"
#include "mcc/encoder/core/ExchangeError.h"
#include "mcc/encoder/core/Packet.h"

namespace mcc { namespace messages { class Message; } }
namespace bmcl { class MemWriter; }

namespace mcc {
namespace encoder {
namespace photon {

    using mcc::encoder::core::ExchangeError;

    struct PacketDetails : public mcc::encoder::core::PacketDetails
    {
        std::size_t _dataOffset;
        std::size_t _dataSize;
    };

    class PhotonCoder
    {
    public:
        using Request = uint8_t;
        using Response = uint8_t;

        static bmcl::Result<std::vector<uint8_t>, ExchangeError> encodePacket(const Request* request);
        static bmcl::Result<PacketDetails, ExchangeError> decodePacket(const void* start, std::size_t size);

        template<typename R = Response>
        static bmcl::Result<R, ExchangeError> decode(const void* start, std::size_t size)
        {
            auto temp_details = decodePacket(start, size);
            if (temp_details.isErr())
                return temp_details.takeErr();

            auto details = temp_details.take();
            bmcl::MemReader packetData((uint8_t*)start + details._dataOffset, details._dataSize);
            return ExchangeError::DecodeNotImplemented;
        }
    };


}
}
}