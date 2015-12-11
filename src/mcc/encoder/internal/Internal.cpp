/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include <QtGlobal>
#include <QDebug>
#include "mcc/messages/Message.h"
#include "mcc/messages/MessageSender.h"
#include "mcc/encoder/internal/Internal.h"

namespace mcc {
namespace encoder {
namespace internal {


bmcl::Result<std::vector<uint8_t>, ExchangeError> InternalCoder::encodePacket(const mcc::messages::Message* request)
{
    return request->serialize();
}

template<>
bmcl::Result<std::unique_ptr<mcc::messages::Message>, ExchangeError> InternalCoder::decode(const void* start, std::size_t size)
{
    bmcl::MemReader reader(start, size);
    if (reader.isEmpty())
        return mcc::encoder::core::ExchangeError::DecodePacketNotEnoughDataForPacket;
    auto p = mcc::messages::Message::deserialize(start, size);
    if (p)
        return std::move(p);
    assert(false);
    return ExchangeError::PacketCantReceive;
}

bmcl::Result<InternalCoder::PacketDetails, ExchangeError> InternalCoder::decodePacket(const void* start, std::size_t size)
{
    Q_UNUSED(start);
    InternalCoder::PacketDetails details;
    details._dataOffset = 0;
    details._dataSize = size;
    details._deviceId = 0;
    details._size = size;
    return details;
}

}
}
}
