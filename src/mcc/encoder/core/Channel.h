/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#pragma once
#include <cstdint>
#include <memory>
#include <thread>
#include <string>

#include "asio/error_code.hpp"
#include "bmcl/MemWriter.h"
#include "mcc/misc/Net.h"

namespace mcc {
namespace encoder {
namespace core {

class Channel
{
public:
    Channel(const std::string& name, const std::string& file_name) : _name(name), _file_name(file_name){}
    virtual ~Channel(){}
    virtual void clear() = 0;
    std::string errorString() const { if (_error) return QString::fromLocal8Bit(_error.message().c_str()).toStdString(); else return ""; };
    virtual bool connect() = 0;
    virtual void disconnect() = 0;
    virtual bool send(const void* ptr, std::size_t size) = 0;
    virtual bool receive(std::vector<uint8_t>* data, std::size_t msTimeout) = 0;
    const std::string& name() const { return _name; };
    const std::string file_name() const { return _file_name; }
    virtual bool isOpen() const = 0;
    mcc::misc::ConnectionState state() const
    {
        return isOpen() ? mcc::misc::ConnectionState::connected : mcc::misc::ConnectionState::disconnected;
    }
    static std::unique_ptr<Channel> makeChannel(const std::string& settings);

protected:
    inline bool isErr() { return (bool)_error; }
    inline asio::error_code& error() { return _error; }

private:
    asio::error_code _error;
    std::string _name;
    std::string _file_name;
};

}
}
}
