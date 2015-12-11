/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#pragma once
#include <memory>
#include <fstream>
#include <iomanip>
#include "bmcl/Option.h"

#include "mcc/encoder/core/Channel.h"
#include "mcc/encoder/core/Packet.h"
#include "mcc/encoder/core/ExchangeError.h"
#include "mcc/messages/Stats.h"


namespace mcc {
namespace encoder {
namespace core {

    class Exchanger
    {
    public:
        Exchanger(const std::string& address, const std::string& settings, const PacketSearcher& searcher, const ChannelOpenerPtr& opener)
            : _searcher(searcher), _address(address), _settings(settings), _opener(opener)
        {
            _rcvd.reset();
            _bad.reset();
            _sent.reset();
        }

        const std::string& address() const { return _address; }
        bool isOpen() const { return _channel && _channel->isOpen(); }

        std::string start()
        {
            if (_channel)
                return "channel already created";

            _channel = mcc::encoder::core::Channel::makeChannel(_settings);
            if (!_channel)
                return "cant create channel";

            return std::string();
        }

        std::string stop()
        {
            if (!_channel)
                return "channel is not created yet";

            _channel->disconnect();
            _channel.reset();
            return std::string();
        }

        std::string open()
        {
            if (!_channel)
                return "connection type not supported";

            _channel->connect();
            if (!_channel->isOpen())
            {
                return "cant connect: " + _channel->errorString();
            }

            _dump.open("io_dump_" + _channel->file_name() + ".txt", std::ios_base::out | std::ios_base::app);
            if (!_opener)
                return std::string();

            auto r = _opener->open(_channel.get());
            if (r.isSome())
            {
                close();
                return r.take();
            }
            return std::string();
        }

        std::string close()
        {
            _dump.close();

            if (!_channel)
                return "channel is not created yet";

            if (!_channel->isOpen())
                return "channel is not opened yet";

            _channel->disconnect();
            return std::string();
        }

        bmcl::Option<PacketDetails> getPacket(std::vector<uint8_t>* out)
        {
            out->clear();
            auto r = _searcher(_buffer.data(), _buffer.size());
            if (r._packet.isSome())
            {
                auto packet = r._packet.take();
                _rcvd.add(packet._size);
                out->assign(_buffer.begin() + r._offset, _buffer.begin() + r._offset + packet._size);
                _buffer.erase(_buffer.begin(), _buffer.begin() + r._offset + packet._size);
                return packet;
            }

            if (r._offset > 0)
                _bad.add(r._offset, false);

            _buffer.erase(_buffer.begin(), _buffer.begin() + r._offset);
            return bmcl::None;
        }

        bool sendData(const std::vector<uint8_t>& in)
        {
            //buf_printer("out: ", in.data(), in.size());
            _sent.add(in.size());
            return _channel->send(in.data(), in.size());
        }

        bool receiveData(std::size_t msTimeout)
        {
            std::size_t offset = _buffer.size();
            bool r = _channel->receive(&_buffer, msTimeout);
            //buf_printer("in: ", _buffer.data()+offset, _buffer.size()-offset);
            return r;
        }

        bmcl::Option<PacketDetails> receivePacket(std::size_t msTimeout, std::vector<uint8_t>* out)
        {
            using namespace std::chrono;
            steady_clock::time_point start = steady_clock::now();
            while (duration_cast<milliseconds>(steady_clock::now() - start).count() <= (long)msTimeout)
            {
                std::size_t offset = _buffer.size();
                _channel->receive(&_buffer, msTimeout);
                //buf_printer("in: ", _buffer.data() + offset, _buffer.size() - offset);
                auto packet = getPacket(out);
                if (packet.isSome())
                    return packet;
            }
            return bmcl::None;
        }

        void clear()
        {
            _channel->clear();
            _buffer.clear();
        }

        const mcc::messages::Stat& statSent() const { return _sent; }
        const mcc::messages::Stat& statRcvd() const { return _rcvd; }
        const mcc::messages::Stat& statBad() const { return _bad; }

    private:
//         void buf_printer(const char* prefix, const void* ptr, std::size_t size)
//         {
//             _dump << prefix;
//             for (std::size_t i = 0; i < size; ++i)
//             {
//                 int j = *((uint8_t*)ptr + i);
//                 _dump << std::hex << std::setw(2) << std::setfill('0') << j << " ";
//             }
//             _dump << std::endl << std::flush;
//         }

    private:
        mcc::messages::Stat _sent;
        mcc::messages::Stat _rcvd;
        mcc::messages::Stat _bad;

        PacketSearcher              _searcher;
        std::vector<uint8_t>        _buffer;
        std::unique_ptr<Channel>    _channel;
        std::string _address;
        std::string _settings;
        ChannelOpenerPtr            _opener;
        std::ofstream _dump;
    };

}
}
}
