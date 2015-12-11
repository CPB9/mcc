/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#pragma once
#include <memory>
#include "asio/ip/udp.hpp"
#include "asio/buffer.hpp"
#include "asio/steady_timer.hpp"
#include "mcc/encoder/core/Channel.h"


namespace mcc {
namespace encoder {
namespace core {

class ChannelUdp : public Channel
{
public:
    ChannelUdp(const mcc::misc::NetUdpParams& address)
        : Channel(address.name().toStdString(), (address.host() + QString::number(address.remotePort().unwrapOr(0))).toStdString()), _address(address), _isRemoteDefined(false)
    {
    }

    ~ChannelUdp()
    {
    }

    bool isOpen() const override { return _socket && _socket->is_open(); }

    bool connect() override
    {
        if (_address.localPort().isNone() && _address.remotePort().isNone())
        {
            error() = asio::error_code(asio::error::host_not_found);
            return false;
        }

        _socket.reset(new asio::ip::udp::socket(_io_service));
        _socket->open(asio::ip::udp::v4(), error());
        if (isErr())
            return false;

        // Resolve the address corresponding to the given host.
        if (_address.remotePort().isSome())
        {
            asio::ip::udp::resolver resolver(_io_service);
            asio::ip::udp::resolver::query query(asio::ip::udp::v4(), _address.host().toStdString(), std::to_string(_address.remotePort().unwrap()));
            asio::ip::udp::resolver::iterator iterator = resolver.resolve(query, error());
            if (isErr())
                return false;
            _socket->connect(iterator->endpoint(), error());
            if (isErr())
                return false;
            _isRemoteDefined = true;
        }

        if (_address.localPort().isSome())
        {
            _socket->bind(asio::ip::udp::endpoint(asio::ip::udp::v4(), _address.localPort().unwrap()), error());
            if (isErr())
                return false;
        }

        return !isErr();
    }

    void disconnect() override
    {
        _isRemoteDefined = false;
        if (!_socket)
            return;
        _socket->close();
        _socket.reset();
        _io_service.stop();
    }

    bool send(const void* ptr, std::size_t size) override
    {
        assert(size != 0);
        if (!_isRemoteDefined)
            return false;
        _socket->send(asio::buffer(ptr, size), 0, error());
        return !isErr();
    }

    bool receive(std::vector<uint8_t>* data, std::size_t msTimeout) override
    {
        enum class Event { none, timeout, message} event = Event::none;
        bool result = false;
        asio::ip::udp::endpoint endpoint;

        asio::steady_timer timer(_io_service);
        timer.expires_from_now(std::chrono::milliseconds(msTimeout));
        timer.async_wait([&](const asio::error_code& error) { event = Event::timeout; });

        _socket->async_wait(asio::ip::udp::socket::wait_read, [&](const asio::error_code& error)
            {
                event = Event::message;
                std::size_t available = _socket->available();
                if (available > 0)
                {
                    std::size_t offset = data->size();
                    data->resize(offset + available);

                    asio::error_code ec;
                    std::size_t size = _socket->receive_from(asio::buffer(data->data() + offset, available), endpoint, 0, ec);

                    data->resize(offset + size);
                }
            });

        // http://stackoverflow.com/questions/10858719/cancel-async-read-due-to-timeout
        while (_io_service.run_one())
        {
            switch (event)
            {
            case Event::none:
                assert(false);
                break;
            case Event::timeout:
                _socket->cancel();
                break;
            case Event::message:
                {
                    result = true;
                    timer.cancel();
                    if (!_isRemoteDefined)
                    {
                        _socket->connect(endpoint, error());
                        if (isErr())
                        {
                            assert(false);
                        }
                        else
                        {
                            _isRemoteDefined = true;
                        }
                    }
                }
                break;
            default:
                assert(false);
                break;
            }
            event = Event::none;
        }
        _io_service.reset();
        return result;
    };

    void clear() override
    {
        //_socket.cancel();
        //_socket.atEnd();
    }

private:
    asio::io_service      _io_service;
    std::unique_ptr<asio::ip::udp::socket> _socket;

    mcc::misc::NetUdpParams _address;
    bool _isRemoteDefined;
};

}
}
}
