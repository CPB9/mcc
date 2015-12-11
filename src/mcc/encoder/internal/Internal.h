/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#pragma once
#include <vector>
#include "bmcl/Result.h"
#include "mcc/messages/MessageSender.h"
#include "mcc/encoder/core/ExchangeError.h"
#include "mcc/encoder/core/Packet.h"

namespace mcc { namespace messages { class Message; } }

namespace mcc {
namespace encoder {
namespace internal {

    using mcc::encoder::core::ExchangeError;

    class InternalCoder
    {
    public:

        struct PacketDetails : public mcc::encoder::core::PacketDetails
        {
            std::size_t  _dataOffset;
            std::size_t  _dataSize;
        };

        using Request = mcc::messages::Message;
        using Response = std::unique_ptr<mcc::messages::Message>;

        static bmcl::Result<std::vector<uint8_t>, ExchangeError> encodePacket(const mcc::messages::Message* request);
        static bmcl::Result<PacketDetails, ExchangeError> decodePacket(const void* start, std::size_t size);

        template<typename R = Response>
        static bmcl::Result<R, ExchangeError> decode(const void* start, std::size_t size);

    private:

    };


}
}
}