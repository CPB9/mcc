/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#pragma once
#include <string>
#include <vector>
#include <atomic>
#include "mcc/messages/Message.h"


namespace mcc {
namespace messages {

class SystemState_Request : public MessageTo
{
public:
    SystemState_Request(const SystemState_Request&) = default;
    SystemState_Request() : MessageTo(mcc::Names::coreManager()){}
    virtual ~SystemState_Request(){}
    MESSAGE_REQUIREMENTS_DECLARATIONS();
private:
};

class SystemState : public MessageTo
{
public:
    SystemState(const SystemState_Request& request, bool isStarted, const std::string& reason = std::string()) : MessageTo(request.sender()), _isStarted(isStarted), _reason(reason){}
    SystemState(bool isStarted, const std::string& reason = std::string()) : MessageTo(mcc::Names::multicast()), _isStarted(isStarted), _reason(reason){}
    virtual ~SystemState(){}
    MESSAGE_REQUIREMENTS_DECLARATIONS();
    bool isStarted() const { return _isStarted; }
    const std::string& reason() const { return _reason; }
private:
    bool _isStarted;
    std::string _reason;
};

class SystemComponentState_Request : public Message
{
public:
    virtual ~SystemComponentState_Request(){}
    MESSAGE_REQUIREMENTS_DECLARATIONS();
private:
};

class SystemComponentState : public MessageTo
{
public:
    SystemComponentState(bool isStarted, const std::string& reason = std::string()) : MessageTo(mcc::Names::coreManager()), _isStarted(isStarted), _reason(reason){}
    virtual ~SystemComponentState(){}
    MESSAGE_REQUIREMENTS_DECLARATIONS();
    bool isStarted() const { return _isStarted; }
    const std::string& reason() const { return _reason; }
private:
    bool _isStarted;
    std::string _reason;
};


}
}

