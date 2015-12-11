/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#pragma once
#include <string>
#include "mcc/messages/Message.h"
#include "mcc/misc/Protocol.h"


namespace mcc {
namespace messages {

class ProtocolList_Request : public MessageTo
{
public:
    ProtocolList_Request() : MessageTo(mcc::Names::coreDb()){}
    virtual ~ProtocolList_Request(){}
    MESSAGE_REQUIREMENTS_DECLARATIONS();
private:
};

class ProtocolList_Response : public Response
{
public:
    ProtocolList_Response(const ProtocolList_Request* request, const std::vector<std::string>& protocols = std::vector<std::string>())
        : Response(request), _protocols(protocols)
    {
    }
    ProtocolList_Response(const ProtocolList_Request* request, std::vector<std::string>&& protocols)
        : Response(request), _protocols(std::move(protocols))
    {
    }
    virtual ~ProtocolList_Response(){}
    MESSAGE_REQUIREMENTS_DECLARATIONS();
    const std::vector<std::string>& protocols()  const { return _protocols; }
private:
    std::vector<std::string> _protocols;
};

class ProtocolDescription_Request : public MessageTo
{
public:
    ProtocolDescription_Request(const std::string& protocol) : MessageTo(mcc::Names::coreDb()), _protocol(protocol)
    {
    }
    virtual ~ProtocolDescription_Request(){}
    MESSAGE_REQUIREMENTS_DECLARATIONS();
    const std::string& protocol() const { return _protocol; }
private:
    std::string _protocol;
};

struct ProtocolDescription
{
    ProtocolDescription() = default;
    ProtocolDescription(std::size_t id, const std::string& name, const std::string& info, const std::string& param_info, const std::string& trait, const std::string& service)
        : _id(id), _name(name), _info(info), _param_info(param_info), _trait(trait), _service(service)
    {
    }
    std::size_t _id;
    std::string _name;
    std::string _info;
    std::string _param_info;
    std::string _trait;
    std::string _service;
};

struct ProtocolParam
{
    ProtocolParam(std::size_t id, const std::string& name, const std::string& info)
        : _id(id), _name(name), _info(info)
    {
    }
    std::size_t _id;
    std::string _name;
    std::string _info;
};
typedef std::vector<ProtocolParam> ProtocolParams;

class ProtocolDescription_Response : public Response
{
public:
    ProtocolDescription_Response(const ProtocolDescription_Request* request
        , const ProtocolDescription& description
        , const std::string& error = std::string())
        : Response(request)
        , _description(description)
        , _error(error)
    {
    }
    ProtocolDescription_Response(const ProtocolDescription_Request* request, const std::string& error) : Response(request), _error(error)
    {
        _description._name = request->protocol();
    }
    virtual ~ProtocolDescription_Response(){}
    MESSAGE_REQUIREMENTS_DECLARATIONS();

    const std::string& protocol() const { return _description._name; }
    const ProtocolDescription& description() const { return _description; }
    const std::string& error() const { return _error; }
private:
    ProtocolDescription _description;
    std::string _error;
};

class ProtocolForDevice_Request : public MessageTo
{
public:
    explicit ProtocolForDevice_Request(const std::string& device) : MessageTo(mcc::Names::coreDb()), _device(device){}
    virtual ~ProtocolForDevice_Request(){}
    MESSAGE_REQUIREMENTS_DECLARATIONS();
    const std::string& device() const { return _device; }
private:
    std::string _device;
};

class ProtocolForDevice_Response : public Response
{
public:
    typedef std::vector<mcc::misc::ProtocolId> Protocols;
    ProtocolForDevice_Response(const ProtocolForDevice_Request* request, const Protocols& protocols) : Response(request), _device(request->device()), _protocols(protocols){}
    ProtocolForDevice_Response(const ProtocolForDevice_Request* request, Protocols&& protocols) : Response(request), _device(request->device()), _protocols(std::move(protocols)){}
    virtual ~ProtocolForDevice_Response(){}
    MESSAGE_REQUIREMENTS_DECLARATIONS();
    const std::string& device() const { return _device; }
    const Protocols& protocols() const { return _protocols; }
private:
    std::string _device;
    Protocols _protocols;
};

class ProtocolForDeviceRegister_Request : public MessageTo
{
public:
    ProtocolForDeviceRegister_Request(const std::string& device, const mcc::misc::ProtocolId& protocol) : MessageTo(mcc::Names::coreDb()), _device(device), _protocol(protocol){}
    virtual ~ProtocolForDeviceRegister_Request(){}
    MESSAGE_REQUIREMENTS_DECLARATIONS();
    const std::string& device() const { return _device; }
    const mcc::misc::ProtocolId& protocol() const { return _protocol; }
private:
    std::string _device;
    mcc::misc::ProtocolId _protocol;
};

class ProtocolForDeviceRegister_Response : public Response
{
public:
    ProtocolForDeviceRegister_Response(const ProtocolForDeviceRegister_Request* request, const std::string& error = std::string())
        : Response(request), _device(request->device()), _protocol(request->protocol()), _error(error){}
    virtual ~ProtocolForDeviceRegister_Response(){}
    MESSAGE_REQUIREMENTS_DECLARATIONS();
    const std::string& device() const { return _device; }
    const mcc::misc::ProtocolId& protocol() const { return _protocol; }
    const std::string& error() const { return _error; }
private:
    std::string _device;
    mcc::misc::ProtocolId _protocol;
    std::string _error;
};

}
}

