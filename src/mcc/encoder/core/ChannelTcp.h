/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#pragma once
#include "asio/ip/tcp.hpp"
#include "asio/buffer.hpp"
#include "asio/steady_timer.hpp"
#include "mcc/encoder/core/Channel.h"


namespace mcc {
namespace encoder {
namespace core {

class ChannelTcp : public Channel
{
public:
    ChannelTcp(const mcc::misc::NetTcpParams& address)
        : Channel(address.name().toStdString(), (address.host() + QString::number(address.remotePort())).toStdString()), _address(address)
    {
    }

    ~ChannelTcp()
    {
    }

    bool isOpen() const override { return _socket && _socket->is_open(); }

    bool connect() override
    {
        // Resolve the address corresponding to the given host.
        asio::ip::tcp::resolver resolver(_io_service);
        asio::ip::tcp::resolver::query query(asio::ip::tcp::v4(), _address.host().toStdString(), std::to_string(_address.remotePort()));
        asio::ip::tcp::resolver::iterator iterator = resolver.resolve(query, error());
        if (isErr())
            return false;

        _socket.reset(new asio::ip::tcp::socket(_io_service));
        _socket->connect(iterator->endpoint(), error());
        if (isErr())
        {
            _socket.reset();
            return false;
        }
        _socket->set_option(asio::socket_base::keep_alive(true), error());
        if (isErr())
        {
            disconnect();
            return false;
        }

        _socket->set_option(asio::ip::tcp::no_delay(true), error());
        if (isErr())
        {
            disconnect();
            return false;
        }
        return true;
    }

    void disconnect() override
    {
        if (!_socket)
            return;

        _socket->shutdown(asio::ip::tcp::socket::shutdown_both, error());
        _socket->close(error());
        _socket.reset();
        _io_service.reset();
        _io_service.stop();

        while (_io_service.run_one())
        {
        }
        _io_service.reset();
    }

    bool send(const void* ptr, std::size_t size) override
    {
        if (size == 0) return false;
        _socket->send(asio::buffer(ptr, size), 0, error());
        return !isErr();
    }

    bool receive(std::vector<uint8_t>* data, std::size_t msTimeout) override
    {
        enum class Event { none, timeout, message } event = Event::none;
        bool result = false;

        asio::steady_timer timer(_io_service);
        timer.expires_from_now(std::chrono::milliseconds(msTimeout));
        timer.async_wait([&](const asio::error_code& error) { event = Event::timeout;  });

        _socket->async_wait(asio::ip::udp::socket::wait_read, [&](const asio::error_code& error)
            {
                event = Event::message;
                std::size_t available = _socket->available();
                if (available > 0)
                {
                    std::size_t offset = data->size();
                    data->resize(offset + available);
                    asio::error_code ec;
                    std::size_t size = _socket->receive(asio::buffer(data->data() + offset, available), 0, ec);
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
    }

private:
    asio::io_service      _io_service;
    std::unique_ptr<asio::ip::tcp::socket> _socket;
    mcc::misc::NetTcpParams _address;
};

}
}
}
