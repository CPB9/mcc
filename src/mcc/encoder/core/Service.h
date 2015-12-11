/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#pragma once
#include <map>
#include <memory>
#include "mcc/messages/ServiceAbstract.h"
#include "mcc/messages/Deaclarations.h"
#include "mcc/misc/Protocol.h"
#include "mcc/misc/Device.h"
#include "mcc/misc/Firmware.h"
#include "mcc/encoder/core/Packet.h"


namespace mcc {
namespace encoder {
namespace core {

class ChannelManager;
class Device;

class Service : public mcc::messages::ServiceAbstract
{
public:
    Service(const std::string& name, const mcc::messages::LocalRouterPtr& router, const std::string& protocolName, const PacketSearcher& packetSearcher, bool syncExchange, const ChannelOpenerPtr& channelOpener = nullptr);
    virtual ~Service();

protected:
    bool pre() override;
    void tick() override;
    void post() override;
    const std::string& protocol() const { return _protocolName; }

protected:
    virtual std::shared_ptr<Device> createDevice(const mcc::misc::ProtocolId& id, const std::string& device_name) = 0;

    std::map<std::string, std::unique_ptr<ChannelManager>>  _channels;
    std::map<std::string, std::shared_ptr<Device>>          _devices;

    std::map <std::string, std::unique_ptr<mcc::messages::DeviceConnect_Request>> _connectRequests;

private:
    void process(std::unique_ptr<mcc::messages::SystemComponentState_Request>&&) override;
    void process(std::unique_ptr<mcc::messages::Cmd>&&) override;
    void process(std::unique_ptr<mcc::messages::CmdCancel>&&) override;
    void process(std::unique_ptr<mcc::messages::DeviceActivate_Request>&&) override;
    void process(std::unique_ptr<mcc::messages::DeviceConnect_Request>&&) override;
    void process(std::unique_ptr<mcc::messages::DeviceDisconnect_Request>&&) override;
    void process(std::unique_ptr<mcc::messages::DeviceFileLoad_Request>&&) override;
    void process(std::unique_ptr<mcc::messages::DeviceFileLoadCancel_Request>&&) override;
    void process(std::unique_ptr<mcc::messages::DeviceList_Response>&&) override;
    void process(std::unique_ptr<mcc::messages::DeviceDescription_Response>&&) override;
    void process(std::unique_ptr<mcc::messages::DeviceUnRegistered>&& notify) override;
    void process(std::unique_ptr<mcc::messages::DeviceUpdate_Response>&& notify) override;
    void process(std::unique_ptr<mcc::messages::DeviceUpdated>&& notify) override;
    void process(std::unique_ptr<mcc::messages::Channel_Request>&& notify) override;
    void process(std::unique_ptr<mcc::messages::FirmwareDescription_Response>&&) override;
    void process(std::unique_ptr<mcc::messages::FirmwareRegister_Response>&&) override;
    void process(std::unique_ptr<mcc::messages::ProtocolForDevice_Response>&&) override;

    void processChannelCreate(std::unique_ptr<mcc::messages::Channel_Request>&& request);
    void processChannelRemove(std::unique_ptr<mcc::messages::Channel_Request>&& request);
    void processChannelOpen(std::unique_ptr<mcc::messages::Channel_Request>&& request);
    void processChannelClose(std::unique_ptr<mcc::messages::Channel_Request>&& request);

    bool _syncExchange;
    std::string _protocolName;
    PacketSearcher _packetSearcher;
    ChannelOpenerPtr _channelOpener;
};
}
}
}
