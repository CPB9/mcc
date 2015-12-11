/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#pragma once
#include <string>
#include "mcc/messages/Message.h"
#include "mcc/messages/Stats.h"


namespace mcc {
namespace messages {

enum Operation
{
    Create,
    Remove,
    Open,
    Close
};

class Channel_Request : public Message
{
public:
    Channel_Request(Operation o, const std::string& address, const std::string& settings = std::string())
        : _operation(o), _address(address), _settings(settings)
    {
    }
    virtual ~Channel_Request(){}
    MESSAGE_REQUIREMENTS_DECLARATIONS();
    Operation operation() const { return _operation; }
    const std::string& address()  const { return _address; }
    const bmcl::Option<std::string>& settings() const { return _settings; }
private:
    Operation _operation;
    std::string _address;
    bmcl::Option<std::string> _settings;
};

class Channel_Response : public Response
{
public:
    Channel_Response(const Channel_Request* request, const std::string& error = std::string())
        : Response(request), _operation(request->operation()), _address(request->address()), _error(error)
    {
    }
    virtual ~Channel_Response(){}
    MESSAGE_REQUIREMENTS_DECLARATIONS();
    Operation operation() const { return _operation; }
    const std::string& address()  const { return _address; }
    const std::string& error()  const { return _error; }
private:
    Operation _operation;
    std::string _address;
    std::string _error;
};

class ChannelState_Response : public Message
{
public:
    ChannelState_Response(const StatChannels& state) :_state(state){}
    ChannelState_Response(StatChannels&& state) :_state(std::move(state)){}
    virtual ~ChannelState_Response(){}
    MESSAGE_REQUIREMENTS_DECLARATIONS();
    const StatChannels& state() const { return _state; }
private:
    StatChannels _state;
};

}
}

