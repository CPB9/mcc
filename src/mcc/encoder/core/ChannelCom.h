/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#pragma once
#include "asio/serial_port.hpp"
#include "asio/steady_timer.hpp"
#include "mcc/encoder/core/Channel.h"


namespace mcc {
namespace encoder {
namespace core {

class ChannelCom : public Channel
{
public:
    ChannelCom(const mcc::misc::NetComParams& address)
        : Channel(address.name().toStdString(), address.portName().toStdString()), _address(address)
    {
    }
    virtual ~ChannelCom()
    {
    }

    bool isOpen() const override { return _port && _port->is_open(); }

    bool connect() override
    {
        _port.reset(new asio::serial_port(_io_service));
        _port->open(_address.portName().toStdString(), error());
        if (isErr())
        {
            _port.reset();
            return false;
        }

        _port->set_option(asio::serial_port_base::baud_rate(_address.baudRate()), error());
        _port->set_option(asio::serial_port_base::character_size(8), error());
        _port->set_option(asio::serial_port_base::flow_control(asio::serial_port_base::flow_control::none), error());
        _port->set_option(asio::serial_port_base::parity(asio::serial_port_base::parity::none), error());
        _port->set_option(asio::serial_port_base::stop_bits(asio::serial_port_base::stop_bits::one), error());
        if (isErr())
        {
            _port.reset();
            return false;
        }
        return true;
    }

    void disconnect() override
    {
        if (!_port)
            return;
        _port->close(error());
        _port.reset();
    }

    bool send(const void* ptr, std::size_t size) override
    {
        if (size == 0) return false;
        _port->write_some(asio::buffer(ptr, size), error());
        return !isErr();
    }

    bool receive(std::vector<uint8_t>* data, std::size_t msTimeout) override
    {
        bool isMessage = false;
        bool isTimeout = false;
        std::vector<uint8_t> buffer;
        buffer.resize(1024);
        asio::error_code ec = asio::error::would_block;

        asio::steady_timer timer(_io_service);
        timer.expires_from_now(std::chrono::milliseconds(msTimeout));
        timer.async_wait([&](const asio::error_code& error) { isTimeout = true; buffer.clear(); });

        _port->async_read_some(asio::buffer(buffer.data(), buffer.size())
                            , [&](const asio::error_code& error, std::size_t bytes_transferred)
                            {
                                if (error != asio::error::would_block && bytes_transferred > 0)
                                {
                                    buffer.resize(bytes_transferred);
                                    data->insert(data->end(), buffer.begin(), buffer.end());
                                    isMessage = true;
                                }
                                else if (error != asio::error::operation_aborted)
                                {
                                    auto msg = error.message();
                                    auto z = error.value();
                                }
                            });

        // http://stackoverflow.com/questions/10858719/cancel-async-read-due-to-timeout
        while (_io_service.run_one())
        {
            if (isMessage)
            {
                timer.cancel();
            }
            else if (isTimeout)
            {
                _port->cancel();
            }
        }
        _io_service.reset();
        return isMessage;
    }

    void clear() override
    {
    }

private:
    asio::io_service      _io_service;
    std::unique_ptr<asio::serial_port> _port;
    mcc::misc::NetComParams _address;
};
}
}
}
