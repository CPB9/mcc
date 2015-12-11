/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include <sstream>

#include "mavlink/pixhawk/mavlink.h"
#include "mavlink/mavlink_helpers.h"

#include "bmcl/Buffer.h"
#include "mcc/misc/Crc.h"
#include "mcc/misc/Channel.h"
#include "mcc/encoder/core/Channel.h"
#include "mcc/encoder/mavlink/Mavlink.h"

namespace mcc {
namespace encoder {
namespace mavlink {

uint8_t MavlinkCoder::version = 3;
uint8_t MavlinkCoder::messageLengthByMessageId[] = MAVLINK_MESSAGE_LENGTHS;
uint8_t MavlinkCoder::crcExtraByMessageId[] = MAVLINK_MESSAGE_CRCS;

bmcl::Result<std::vector<uint8_t>, ExchangeError> MavlinkCoder::encodePacket(const MavlinkMessage* request)
{
    static uint8_t counter = 0;
    std::vector<uint8_t> packet;
    packet.resize(300);
    bmcl::MemWriter writer(packet.data(), packet.size());
    writer.writeUint8(0xFE);
    writer.writeUint8(request->_data.size()); //Payload length
    writer.writeUint8(++counter); //Packet sequence
    writer.writeUint8(request->deviceId()); //System ID
    writer.writeUint8(request->componentId()); //Component ID
    writer.writeUint8(request->_messageId); //Message ID
    writer.write(request->_data.data(), request->_data.size()); //Data
    writer.writeUint16Le(computeCrc(request->_messageId, writer.start() + 1, writer.sizeUsed() - 1));
    packet.resize(writer.sizeUsed());
    return std::move(packet);
}

bmcl::Result<PacketDetails, ExchangeError> MavlinkCoder::decodePacket(const void* start, std::size_t size)
{
    bmcl::MemReader packetReader(start, size);

    if (packetReader.size() < 8)
        return ExchangeError::DecodePacketNotEnoughDataForPacket;

    if (packetReader.readUint8() != 0xFE)
        return ExchangeError::DecodeBadStartSequence;

    PacketDetails details;
    details._dataSize = packetReader.readUint8();
    details._size = details._dataSize + 8;
    if (packetReader.size() < details._size)
        return ExchangeError::DecodePacketNotEnoughDataForPacket;

    details._sequence = packetReader.readUint8();
    details._deviceId = packetReader.readUint8();
    details._componentId = packetReader.readUint8();
    details._messageId = packetReader.readUint8();
    details._dataOffset = packetReader.sizeRead();

    if (details._dataSize != messageLengthByMessageId[details._messageId])
    {
        if (failedMessages.find(details._messageId) != failedMessages.end())
        {
            failedMessages.insert(details._messageId);
            qDebug() << "Invalid message length for " << details._messageId
                << "(" << details._dataSize << "!=" << messageLengthByMessageId[details._messageId] << ")";
        }
        return ExchangeError::DecodeBadDataLength;
    }

    packetReader.skip(details._dataSize);

    //qDebug() << "DeviceId: " << details._deviceId.unwrap() << "ComponentId:" << details._componentId << "MessageId:" << details._messageId;

    if (!computeAndCheckCrc(details._messageId, static_cast<const uint8_t*>(start) + 1,
                            MAVLINK_CORE_HEADER_LEN + details._dataSize, packetReader.readUint16Le()))
    {
        return ExchangeError::DecodeBadCrc;
    }

    return details;
}

uint16_t MavlinkCoder::computeCrc(uint8_t messageId, const uint8_t * start, uint16_t size)
{
    uint16_t computedCrc = crc_calculate(start, size);
#if MAVLINK_CRC_EXTRA
    crc_accumulate(crcExtraByMessageId[messageId], &computedCrc);
#endif
    return computedCrc;
}

bool MavlinkCoder::computeAndCheckCrc(uint8_t messageId, const uint8_t *start, uint16_t size, uint16_t packetCrc)
{
    uint16_t computedCrc = computeCrc(messageId, start, size);
    if (packetCrc != computedCrc)
    {
        qDebug() << "Computed checksum fail: " << computedCrc;
        std::vector<uint8_t> packetData(start, start + size);
        qDebug() << "Original data: (0xfe)" << QString::fromStdString(std::to_string(packetData)) << "(crc: " << packetCrc << ")";
        return false;
    }
    return true;
}

}
}
}

namespace std
{
    string to_string(vector<uint8_t> &vector)
    {
        ostringstream str(ios_base::ate);
        str << "{" << std::hex << std::showbase;
        for (auto it(vector.begin()); it != vector.end(); ++it)
        {
            str << static_cast<int>(*it) << ", ";
        }
        str << "}";
        return str.str();
    }
}