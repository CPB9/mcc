/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#pragma once
#include <vector>
#include <cassert>
#include <set>
#include <QDebug>
#include "bmcl/Result.h"
#include "mcc/misc/Channel.h"
#include "mcc/encoder/core/ExchangeError.h"
#include "mcc/encoder/core/Packet.h"

namespace mcc { namespace messages { class Message; } }
namespace bmcl { class MemWriter; }

namespace mcc {
namespace encoder {
namespace mavlink {

using mcc::encoder::core::ExchangeError;

/*!
 * Список сообщений с ошибками, позволяет не дублировать сообщения об ошибках разбора сообщений.
 */
static std::set<uint8_t> failedMessages;

struct PacketDetails : public mcc::encoder::core::PacketDetails
{
    uint8_t     _sequence;
    uint8_t     _componentId;
    uint8_t     _messageId;
    uint16_t    _version;
    std::size_t _dataOffset;
    std::size_t _dataSize;
};

class MavlinkMessage
{
private:
    core::DeviceId _deviceId;
    uint8_t _componentId;

public:
    uint8_t  _messageId;
    uint16_t _version;
    std::vector<uint8_t> _data;

public:
    MavlinkMessage(uint8_t deviceId, uint8_t componentId) :_deviceId(deviceId), _componentId(componentId){}
    MavlinkMessage(const PacketDetails& details) :_deviceId(details._deviceId.unwrap()), _componentId(details._componentId), _messageId(details._messageId), _version(details._version){}
    core::DeviceId deviceId() const { return _deviceId; }
    uint8_t componentId() const { return _componentId; }

    virtual bmcl::Option<ExchangeError> encode(bmcl::MemWriter* writer) const
    {
        //упаковать поля сообщения
        return ExchangeError::EncodeNotImplemented;
    }
    virtual bmcl::Option<ExchangeError> decode(bmcl::MemReader reader)
    {
        Q_UNUSED(reader);
        //нужно сравнить версии сообщений _version (текущего сообщения) и версию описания из нашей бд
        _data.clear();
        _data.assign(reader.start(), reader.start() + reader.size());
        return bmcl::None;
    }

    virtual void debugPrint() const
    {
        //распечатать сообщение
        //qDebug() << _deviceId << _componentId << _messageId;
        assert(_messageId != 72);
    }
};

class MavlinkCoder
{
public:
    using Request = MavlinkMessage;
    using Response = MavlinkMessage;

    static uint8_t version;
    static uint8_t messageLengthByMessageId[];
    static uint8_t crcExtraByMessageId[];

    static bmcl::Result<std::vector<uint8_t>, ExchangeError> encodePacket(const MavlinkMessage* request);
    static bmcl::Result<PacketDetails, ExchangeError> decodePacket(const void* start, std::size_t size);

    template<typename R = MavlinkMessage>
    static bmcl::Result<R, ExchangeError> decode(const void* start, std::size_t size)
    {
        auto temp_details = decodePacket(start, size);
        if (temp_details.isErr())
            return temp_details.takeErr();

        auto details = temp_details.take();
        bmcl::MemReader packetData((uint8_t*)start + details._dataOffset, details._dataSize);

        R response(details);
        auto r = response.decode(packetData);
        if (r.isNone())
        {
            response.debugPrint();
            return response;
        }
        return r.take();
    }

    static uint16_t computeCrc(uint8_t messageId, const uint8_t * start, uint16_t size);

private:
    static bool computeAndCheckCrc(uint8_t messageId, const uint8_t *start, uint16_t size, uint16_t packetCrc);
};

}
}
}

namespace std
{
string to_string(vector<uint8_t>& vector);
}