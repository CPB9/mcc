/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#pragma once
#include <string>
#include <vector>
#include "mcc/misc/NetVariant.h"
#include "mcc/messages/Message.h"


namespace mcc { namespace protobuf { class TmParamList; } }

namespace mcc {
namespace messages {


class TmParamSubscribe_Request : public MessageTo
{
public:
    TmParamSubscribe_Request(bool isOn, const std::string& device, const std::string& trait = std::string(), const std::string& status = std::string())
        : MessageTo(mcc::Names::coreTm()), _isOn(isOn), _device(device), _trait(trait), _status(status)
    {
    }
    virtual ~TmParamSubscribe_Request(){}
    MESSAGE_REQUIREMENTS_DECLARATIONS();
    bool isOn() const { return _isOn; }
    const std::string& device() const { return _device; }
    const std::string& trait() const { return _trait; }
    const std::string& status() const { return _status; }

private:
    bool _isOn;
    std::string _device;
    std::string _trait;
    std::string _status;
};

class TmParamSubscribe_Response : public Response
{
public:
    TmParamSubscribe_Response(const TmParamSubscribe_Request* request, const std::string& error = std::string())
        : Response(request), _isOn(request->isOn()), _device(request->device()), _trait(request->trait()), _status(request->status()), _error(error)
    {
    }
    virtual ~TmParamSubscribe_Response(){}
    MESSAGE_REQUIREMENTS_DECLARATIONS();
    bool isOn() const { return _isOn; }
    const std::string& device() { return _device; }
    const std::string& trait()  { return _trait; }
    const std::string& status() { return _status; }
    const std::string& error() { return _error; }

private:
    bool _isOn;
    std::string _device;
    std::string _trait;
    std::string _status;
    std::string _error;
};

class TmParam
{
public:
    TmParam(){}
    TmParam(const std::string& trait, const std::string& status, const mcc::misc::NetVariant& value)
        : _trait(trait), _status(status), _value(value)
    {
    }
    const std::string& trait() const { return _trait; }
    const std::string& status() const { return _status; };
    const mcc::misc::NetVariant& value() const { return _value; };
    void set_value(const mcc::misc::NetVariant& v) { _value = v; };

private:
    std::string _trait;
    std::string _status;
    mcc::misc::NetVariant _value;
};
typedef std::vector<TmParam> TmParams;

class TmParamList: public MessageTo
{
public:
    TmParamList(const std::string& device, const TmParams& params = TmParams())
        : MessageTo(mcc::Names::coreTm()), _device(device), _params(params)
    {
    }
    TmParamList(const std::string& device, TmParams&& params)
        : MessageTo(mcc::Names::coreTm()), _device(device), _params(std::move(params))
    {
    }
    virtual ~TmParamList(){}
    static std::unique_ptr<Message> deserialize(const mcc::protobuf::TmParamList& cmd);
    MESSAGE_REQUIREMENTS_DECLARATIONS();

    const std::string& device() const { return _device; }
    const TmParams params() const { return _params; }

protected:
    void serialize_(mcc::protobuf::MessageBody* body) const override;

private:
    std::string _device;
    TmParams    _params;
};



}
}

