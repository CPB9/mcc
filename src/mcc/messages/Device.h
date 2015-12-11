/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#pragma once
#include <string>
#include "mcc/messages/Message.h"
#include "mcc/messages/Stats.h"


namespace mcc {
namespace messages {

class DeviceActivate_Request : public Message
{
public:
    DeviceActivate_Request(const std::string& device, bool isActive)
        : _isActive(isActive), _device(device)
    {
    }
    virtual ~DeviceActivate_Request(){}
    MESSAGE_REQUIREMENTS_DECLARATIONS();
    const std::string& device()  const { return _device; }
    bool isActive() const { return _isActive; }
private:
    bool _isActive;
    std::string _device;
};

class DeviceActivate_Response : public Response
{
public:
    DeviceActivate_Response(const DeviceActivate_Request* request, const std::string& error = std::string())
        : Response(request), _isActive(request->isActive()), _device(request->device()), _error(error)
    {
    }
    virtual ~DeviceActivate_Response(){}
    MESSAGE_REQUIREMENTS_DECLARATIONS();
    const std::string& device()  const { return _device; }
    bool isActive() const { return _isActive; }
    const std::string& error() const { return _error; }
private:
    bool _isActive;
    std::string _device;
    std::string _error;
};

class DeviceConnect_Request : public Message
{
public:
    DeviceConnect_Request(const std::string& device, const std::string& channel)
        : _device(device), _channel(channel)
    {
    }
    virtual ~DeviceConnect_Request(){}
    MESSAGE_REQUIREMENTS_DECLARATIONS();
    const std::string& device()  const { return _device; }
    const std::string& channel()  const { return _channel; }
private:
    std::string _device;
    std::string _channel;
};

class DeviceConnect_Response : public Response
{
public:
    DeviceConnect_Response(const DeviceConnect_Request* request, const std::string& error = std::string())
        : Response(request), _device(request->device()), _channel(request->channel()), _error(error)
    {
    }
    virtual ~DeviceConnect_Response(){}
    MESSAGE_REQUIREMENTS_DECLARATIONS();
    const std::string& device()  const { return _device; }
    const std::string& channel()  const { return _channel; }
    const std::string& error() const { return _error; }
private:
    std::string _device;
    std::string _channel;
    std::string _error;
};

class DeviceDisconnect_Request : public Message
{
public:
    DeviceDisconnect_Request(const std::string& device, const std::string& channel)
        : _device(device), _channel(channel)
    {
    }
    virtual ~DeviceDisconnect_Request(){}
    MESSAGE_REQUIREMENTS_DECLARATIONS();
    const std::string& device()  const { return _device; }
    const std::string& channel()  const { return _channel; }
private:
    std::string _device;
    std::string _channel;
};

class DeviceDisconnect_Response : public Response
{
public:
    DeviceDisconnect_Response(const DeviceDisconnect_Request* request, const std::string& error = std::string())
        : Response(request), _device(request->device()), _channel(request->channel()), _error(error)
    {
    }
    virtual ~DeviceDisconnect_Response(){}
    MESSAGE_REQUIREMENTS_DECLARATIONS();
    const std::string& device()  const { return _device; }
    const std::string& channel()  const { return _channel; }
    const std::string& error() const { return _error; }
private:
    std::string _device;
    std::string _channel;
    std::string _error;
};

class DeviceRegister_Request : public MessageTo
{
public:
    DeviceRegister_Request(const std::string& info)
        : MessageTo(mcc::Names::coreDb()), _info(info)
    {
    }
    virtual ~DeviceRegister_Request(){}
    MESSAGE_REQUIREMENTS_DECLARATIONS();
    const std::string& info()  const { return _info; }
private:
    std::string _info;
};

class DeviceRegister_Response : public Response
{
public:
    DeviceRegister_Response(const DeviceRegister_Request* request, std::size_t id, const std::string& name)
        : Response(request), _id(id), _name(name), _info(request->info())
    {
    }
    DeviceRegister_Response(const DeviceRegister_Request* request, const std::string& error = std::string())
        : Response(request), _id(-1), _info(request->info()), _error(error)
    {
    }
    virtual ~DeviceRegister_Response(){}
    MESSAGE_REQUIREMENTS_DECLARATIONS();
    const std::string& name()  const { return _name; }
    const std::size_t& id()  const { return _id; }
    const std::string& info()  const { return _info; }
    const std::string& error() const { return _error; }
private:
    std::size_t _id;
    std::string _name;
    std::string _info;
    std::string _error;
};

class DeviceUnRegister_Request : public MessageTo
{
public:
    DeviceUnRegister_Request(const std::string& device)
        : MessageTo(mcc::Names::coreDb()), _device(device)
    {
    }
    virtual ~DeviceUnRegister_Request(){}
    MESSAGE_REQUIREMENTS_DECLARATIONS();
    const std::string& device()  const { return _device; }
private:
    std::string _device;
};

class DeviceUnRegister_Response : public Response
{
public:
    DeviceUnRegister_Response(const DeviceUnRegister_Request* request, const std::string& error = std::string())
        : Response(request), _device(request->device()), _error(error)
    {
    }
    virtual ~DeviceUnRegister_Response(){}
    MESSAGE_REQUIREMENTS_DECLARATIONS();
    const std::string& device()  const { return _device; }
    const std::string& error() const { return _error; }
private:
    std::string _device;
    std::string _error;
};

class DeviceRegistered : public MessageTo
{
public:
    DeviceRegistered(std::size_t id, const std::string& name, const std::string& info)
        : MessageTo(mcc::Names::multicast()), _id(id), _name(name), _info(info)
    {
    }
    virtual ~DeviceRegistered(){}
    MESSAGE_REQUIREMENTS_DECLARATIONS();
    std::size_t id() const { return _id; }
    const std::string& name()  const { return _name; }
    const std::string& info()  const { return _info; }
private:
    std::size_t _id;
    std::string _name;
    std::string _info;
};

class DeviceUnRegistered : public MessageTo
{
public:
    DeviceUnRegistered(const std::string& device) : MessageTo(mcc::Names::multicast()), _device(device) {}
    virtual ~DeviceUnRegistered(){}
    MESSAGE_REQUIREMENTS_DECLARATIONS();
    const std::string& device()  const { return _device; }
private:
    std::string _device;
};

class DeviceUpdate_Request : public MessageTo
{
public:
    DeviceUpdate_Request(const std::string& device, const std::string& firmware, const std::string& kind)
        : MessageTo(mcc::Names::coreDb()), _device(device), _firmware(firmware), _kind(kind){}
    virtual ~DeviceUpdate_Request(){}
    MESSAGE_REQUIREMENTS_DECLARATIONS();
    const std::string& device()  const { return _device; }
    const std::string& firmware()  const { return _firmware; }
    const std::string& kind()  const { return _kind; }
private:
    std::string _device;
    std::string _firmware;
    std::string _kind;
};

class DeviceUpdate_Response : public Response
{
public:
    DeviceUpdate_Response(const DeviceUpdate_Request* request, const std::string& error = std::string())
        : Response(request), _device(request->device()), _firmware(request->firmware()), _error(error){}
    virtual ~DeviceUpdate_Response(){}
    MESSAGE_REQUIREMENTS_DECLARATIONS();
    const std::string& device()  const { return _device; }
    const std::string& firmware()  const { return _firmware; }
    const std::string& error() const { return _error; }
private:
    std::string _device;
    std::string _firmware;
    std::string _error;
};

class DeviceUpdated : public MessageTo
{
public:
    DeviceUpdated(const std::string& device) : MessageTo(mcc::Names::multicast()), _device(device){}
    virtual ~DeviceUpdated(){}
    MESSAGE_REQUIREMENTS_DECLARATIONS();
    const std::string& device()  const { return _device; }
private:
    std::string _device;
};

class DeviceFileLoad_Request : public Message
{
public:
    enum Direction
    {
        Up,
        Down
    };
    DeviceFileLoad_Request(const std::string& device, const std::string& file_path, Direction dir)
        : _device(device), _file_path(file_path), _dir(dir)
    {
    }
    virtual ~DeviceFileLoad_Request(){}
    MESSAGE_REQUIREMENTS_DECLARATIONS();
    const std::string& device()  const { return _device; }
    const std::string& file_path()  const { return _file_path; }
    Direction direction()  const { return _dir; }
private:
    std::string _device;
    std::string _file_path;
    Direction   _dir;
};

class DeviceFileLoadCancel_Request : public Message
{
public:
    DeviceFileLoadCancel_Request(const std::string& device, const std::string& file_path, const std::string& reason)
        : _device(device), _file_path(file_path), _reason(reason)
    {
    }
    virtual ~DeviceFileLoadCancel_Request(){}
    MESSAGE_REQUIREMENTS_DECLARATIONS();
    const std::string& device()  const { return _device; }
    const std::string& file_path()  const { return _file_path; }
    const std::string& reason()  const { return _reason; }
private:
    std::string _device;
    std::string _file_path;
    std::string _reason;
};

class DeviceFileLoad_Response : public Response
{
public:
    DeviceFileLoad_Response(const DeviceFileLoad_Request* request, const std::string& error = std::string())
        : Response(request), _device(request->device()), _file_path(request->file_path()), _error(error)
    {
    }
    DeviceFileLoad_Response(const DeviceFileLoadCancel_Request* request, const std::string& error = std::string())
        : Response(request), _device(request->device()), _file_path(request->file_path()), _error(error)
    {
    }
    virtual ~DeviceFileLoad_Response(){}
    MESSAGE_REQUIREMENTS_DECLARATIONS();
    const std::string& device()  const { return _device; }
    const std::string& file_path()  const { return _file_path; }
    const std::string& error()  const { return _error; }
private:
    std::string _device;
    std::string _file_path;
    std::string _error;
};

class DeviceList_Request : public MessageTo
{
public:
    DeviceList_Request() : MessageTo(mcc::Names::coreDb()) {}
    virtual ~DeviceList_Request(){}
    MESSAGE_REQUIREMENTS_DECLARATIONS();
private:
};

class DeviceList_Response : public Response
{
public:
    DeviceList_Response(const DeviceList_Request* request, const std::vector<std::string>& devices) : Response(request), _devices(devices){}
    DeviceList_Response(const DeviceList_Request* request, std::vector<std::string>&& devices) : Response(request), _devices(std::move(devices)){}
    virtual ~DeviceList_Response(){}
    MESSAGE_REQUIREMENTS_DECLARATIONS();
    const std::vector<std::string>& devices() const { return _devices; }
private:
    std::vector<std::string> _devices;
};

class DeviceDescription_Request : public MessageTo
{
public:
    DeviceDescription_Request(const std::string& device) : MessageTo(mcc::Names::coreDb()), _device(device){}
    virtual ~DeviceDescription_Request(){}
    MESSAGE_REQUIREMENTS_DECLARATIONS();
    const std::string& device() const { return _device; }
private:
    std::string _device;
};

class DeviceDescription_Response : public Response
{
public:
    DeviceDescription_Response(const DeviceDescription_Request* request, const std::string& error)
        : Response(request), _device_name(request->device()), _error(error)
    {
    }

    DeviceDescription_Response(const DeviceDescription_Request* request
                                , std::size_t device_id
                                , const std::string& device_info
                                , std::size_t kind_id
                                , const std::string& kind_name
                                , const std::string& kind_info
                                , std::size_t firmware_id
                                , const std::string& firmware_name
                                , const std::string& firmware_info)
        : Response(request)
        , _device_name(request->device())
        , _device_id(device_id)
        , _device_info(device_info)
        , _kind_id(kind_id)
        , _kind_name(kind_name)
        , _kind_info(kind_info)
        , _firmware_id(firmware_id)
        , _firmware_name(firmware_name)
        , _firmware_info(firmware_info)
    {
    }
    virtual ~DeviceDescription_Response(){}
    MESSAGE_REQUIREMENTS_DECLARATIONS();
    const std::string& device() const { return _device_name; }
    const std::string& error() const { return _error; }

    std::size_t device_id() const { return _device_id; }
    const std::string&  device_info() const { return _device_info; }

    std::size_t  kind_id() const { return _kind_id; }
    const std::string&  kind_name() const { return _kind_name; }
    const std::string&  kind_info() const { return _kind_info; }

    std::size_t firmware_id() const { return _firmware_id; }
    const std::string&  firmware_name() const { return _firmware_name; }
    const std::string&  firmware_info() const { return _firmware_info; }
private:
    std::string _error;

    std::size_t _device_id;
    std::string _device_name;
    std::string _device_info;

    std::size_t _kind_id;
    std::string _kind_info;
    std::string _kind_name;

    std::size_t _firmware_id;
    std::string _firmware_name;
    std::string _firmware_info;
};

class DeviceState_Response : public Message
{
public:
    DeviceState_Response(const StatDevices& state) :_state(state){}
    DeviceState_Response(StatDevices&& state) :_state(std::move(state)){}
    virtual ~DeviceState_Response(){}
    MESSAGE_REQUIREMENTS_DECLARATIONS();
    const StatDevices& state() const { return _state; }
private:
    StatDevices _state;
};

class DeviceActionLog : public MessageTo
{
public:
    DeviceActionLog(const std::string& kind, const std::string& name, const std::string& action, const std::string& details = std::string())
        : MessageTo(mcc::Names::coreDb()), _kind(kind), _name(name), _action(action), _details(details){}
    virtual ~DeviceActionLog(){}
    MESSAGE_REQUIREMENTS_DECLARATIONS();
    const std::string& kind() const { return _kind; }
    const std::string& name() const { return _name; }
    const std::string& action() const { return _action; }
    const std::string& details() const { return _details; }
private:
    std::string _kind;
    std::string _name;
    std::string _action;
    std::string _details;
};

}
}

