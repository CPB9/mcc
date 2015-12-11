/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#pragma once
#include "mcc/messages/ServiceAbstract.h"
#include "mcc/core/db/DbHandle.h"

#include "mcc/core/decode/Sqlite3RegistryProvider.h"

namespace mcc { namespace core { namespace db { namespace queries
{
    class Cmd;
    class CmdState;
    class TmParamList;
    class DeviceActionLog;
    class DeviceDescription_Request;
    class DeviceList_Request;
    class FirmwareList_Request;
    class ProtocolForDevice_Request;
    class ProtocolForDeviceRegister_Request;
    class ProtocolList_Request;
    class ProtocolDescription_Request;
}}}}


namespace mcc {
namespace core {
namespace db {

class Service : public mcc::messages::ServiceAbstract
{
public:
    Service(const mcc::messages::LocalRouterPtr& router);
    virtual ~Service();

protected:
    bool pre() override;
    void post() override;
    void tick() override;

private:
    void process(std::unique_ptr<mcc::messages::Cmd>&&) override;
    void process(std::unique_ptr<mcc::messages::CmdState>&&) override;
    void process(std::unique_ptr<mcc::messages::DeviceDescription_Request>&&) override;
    void process(std::unique_ptr<mcc::messages::DeviceUpdate_Request>&&) override;
    void process(std::unique_ptr<mcc::messages::DeviceList_Request>&&) override;
    void process(std::unique_ptr<mcc::messages::DeviceRegister_Request>&&) override;
    void process(std::unique_ptr<mcc::messages::DeviceUnRegister_Request>&&) override;
    void process(std::unique_ptr<mcc::messages::DeviceActionLog>&&) override;
    void process(std::unique_ptr<mcc::messages::FirmwareDescription_Request>&&) override;
    void process(std::unique_ptr<mcc::messages::FirmwareList_Request>&&) override;
    void process(std::unique_ptr<mcc::messages::FirmwareRegister_Request>&&) override;
    void process(std::unique_ptr<mcc::messages::ProtocolDescription_Request>&&) override;
    void process(std::unique_ptr<mcc::messages::ProtocolForDevice_Request>&&) override;
    void process(std::unique_ptr<mcc::messages::ProtocolForDeviceRegister_Request>&&) override;
    void process(std::unique_ptr<mcc::messages::ProtocolList_Request>&&) override;
    void process(std::unique_ptr<mcc::messages::TmParamList>&&) override;

    std::unique_ptr<queries::Cmd> _cmd;
    std::unique_ptr<queries::CmdState> _cmdstate;
    std::unique_ptr<queries::TmParamList> _tmparamlist;
    std::unique_ptr<queries::DeviceActionLog> _actions;
    std::unique_ptr<queries::DeviceDescription_Request> _devicedescription;
    std::unique_ptr<queries::DeviceList_Request> _devicelist;
    std::unique_ptr<queries::ProtocolForDevice_Request> _protocolfordevice;
    std::unique_ptr<queries::ProtocolForDeviceRegister_Request> _protocolfordeviceregister;
    std::unique_ptr<queries::ProtocolList_Request> _protocollist;
    std::unique_ptr<queries::ProtocolDescription_Request> _protocoldescription;
    std::unique_ptr<queries::FirmwareList_Request> _firmwarelist;

    const std::string _dbSchema = ":/db/Schema.sql";
    const std::string _dirTraits = ":/db/traits/";
    mcc::core::db::DbHandle _db;
    std::unique_ptr<mcc::decode::Registry> _registry;
};
}
}
}