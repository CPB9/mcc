/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#pragma once
#include <string>
#include "mcc/messages/Message.h"
#include "mcc/misc/Firmware.h"


namespace mcc {
namespace messages {

class FirmwareList_Request : public MessageTo
{
public:
    FirmwareList_Request () : MessageTo(mcc::Names::coreDb()){}
    virtual ~FirmwareList_Request(){}
    MESSAGE_REQUIREMENTS_DECLARATIONS();
private:
};

class FirmwareList_Response : public Response
{
public:
    FirmwareList_Response(const FirmwareList_Request* request, const std::vector<std::string>& firmwares = std::vector<std::string>())
        : Response(request), _firmwares(firmwares)
    {
    }
    FirmwareList_Response(const FirmwareList_Request* request, std::vector<std::string>&& firmwares)
        : Response(request), _firmwares(std::move(firmwares))
    {
    }
    virtual ~FirmwareList_Response(){}
    MESSAGE_REQUIREMENTS_DECLARATIONS();
    const std::vector<std::string>& firmwares()  const { return _firmwares; }
private:
    std::vector<std::string> _firmwares;
};

class FirmwareDescription_Request : public MessageTo
{
public:
    FirmwareDescription_Request(const std::string& firmware) : MessageTo(mcc::Names::coreDb()), _firmware(firmware){}
    virtual ~FirmwareDescription_Request(){}
    MESSAGE_REQUIREMENTS_DECLARATIONS();
    const std::string& firmware() const { return _firmware; }
private:
    std::string _firmware;
};

class FirmwareDescription_Response: public Response
{
public:
    FirmwareDescription_Response(const FirmwareDescription_Request* request, const mcc::misc::FirmwareDescriptionPtr& description, const std::string& error = std::string())
        : Response(request), _firmware(request->firmware()), _description(description), _error(error){}
    virtual ~FirmwareDescription_Response(){}
    MESSAGE_REQUIREMENTS_DECLARATIONS();
    const std::string& firmware() const { return _firmware; }
    const std::string& error() const { return _error; }
    const mcc::misc::FirmwareDescriptionPtr& description() const { return _description; }
private:
    std::string _firmware;
    std::string _error;
    mcc::misc::FirmwareDescriptionPtr _description;
};

class FirmwareRegister_Request : public MessageTo
{
public:
    FirmwareRegister_Request(const std::string& source, const std::string& name, const std::string& info, const mcc::misc::FirmwareDescriptionPtr& firmware)
        : MessageTo(mcc::Names::coreDb()), _source(source), _name(name), _info(info), _firmware(firmware){}
    virtual ~FirmwareRegister_Request(){}
    MESSAGE_REQUIREMENTS_DECLARATIONS();
    const std::string& source() const { return _source; }
    const std::string& name() const { return _name; }
    const std::string& info() const { return _info; }
    const mcc::misc::FirmwareDescriptionPtr& firmware() const { return _firmware; }
private:
    std::string _source;
    std::string _name;
    std::string _info;
    mcc::misc::FirmwareDescriptionPtr _firmware;
};

class FirmwareRegister_Response : public Response
{
public:
    FirmwareRegister_Response(const FirmwareRegister_Request* request, const std::string& error)
        : Response(request), _source(request->source()), _name(request->name()), _id(-1), _error(error)
    {
    }
    FirmwareRegister_Response(const FirmwareRegister_Request* request, std::size_t id)
        : Response(request), _source(request->source()), _name(request->name()), _id(id)
    {
    }
    virtual ~FirmwareRegister_Response(){}
    MESSAGE_REQUIREMENTS_DECLARATIONS();
    const std::string& source() const { return _source; }
    const std::string& name() const { return _name; }
    std::size_t id() const { return _id; }
    const std::string& error() const { return _error; }
private:
    std::string _source;
    std::string _name;
    std::size_t _id;
    std::string _error;
};

class FirmwareRegistered : public MessageTo
{
public:
    FirmwareRegistered(const std::string& name, std::size_t id, bool isRegistered) : MessageTo(mcc::Names::multicast()), _name(name), _id(id), _isRegistered(isRegistered) {}
    virtual ~FirmwareRegistered(){}
    MESSAGE_REQUIREMENTS_DECLARATIONS();
    const std::string& name()  const { return _name; }
    std::size_t id() const { return _id; }
    bool isRegistered() const { return _isRegistered; }
private:
    bool _isRegistered;
    std::size_t _id;
    std::string _name;
};


}
}

