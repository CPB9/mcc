/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#pragma once
#include <functional>
#include <memory>
#include <string>
#include "bmcl/MemReader.h"
#include "bmcl/Option.h"
#include "mcc/encoder/core/ExchangeError.h"

namespace mcc {
namespace encoder {
namespace core {

typedef std::size_t DeviceId;

struct PacketDetails
{
    std::size_t _size;
    bmcl::Option<DeviceId> _deviceId;
};

struct SearchResult
{
    SearchResult(){}
    explicit SearchResult(std::size_t offset) : _offset(offset){}
    SearchResult(std::size_t offset, const PacketDetails& packet) : _offset(offset), _packet(packet){}
    std::size_t _offset;
    bmcl::Option<PacketDetails> _packet;
};

template <typename T>
static inline bmcl::Option<ExchangeError> validatePacket(const void* start, std::size_t size)
{
    auto r = T::decodePacket(start, size);
    if (r.isOk())
        return bmcl::None;
    return r.takeErr();
};

template <typename T>
static inline mcc::encoder::core::SearchResult findPacket(const void* start, std::size_t size)
{
    if (size == 0)
        return mcc::encoder::core::SearchResult(0);

    bmcl::MemReader reader(start, size);

    while (!reader.isEmpty())
    {
        auto r = T::decodePacket(reader.current(), reader.sizeLeft());
        if (r.isOk())
            return mcc::encoder::core::SearchResult(reader.sizeRead(), r.take());

        auto err = r.takeErr();
        if (err == ExchangeError::DecodePacketNotEnoughDataForPacket)
            return mcc::encoder::core::SearchResult(reader.sizeRead());

        reader.skip(1);
    }

    return mcc::encoder::core::SearchResult(size);
}

class PacketSearcher
{
    typedef std::function<SearchResult(const void* start, std::size_t size)> Searcher;
public:
    PacketSearcher(const Searcher& searcher) :_searcher(searcher){}
    inline SearchResult operator()(const void* start, std::size_t size) const
    {
        return _searcher(start, size);
    }
private:
    Searcher _searcher;
};

class Channel;

class ChannelOpener
{
public:
    virtual bmcl::Option<std::string> open(Channel* channel) = 0;
    ~ChannelOpener(){}
};
typedef std::shared_ptr<ChannelOpener> ChannelOpenerPtr;

}
}
}
