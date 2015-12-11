/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#pragma once
#include <memory>

#include "bmcl/MemReader.h"
#include "bmcl/MemWriter.h"
#include "bmcl/Result.h"

#include "mcc/misc/Net.h"
#include "mcc/misc/Device.h"
#include "mcc/misc/Protocol.h"
#include "mcc/misc/Channel.h"
#include "mcc/misc/Cmd.h"
#include "mcc/misc/Firmware.h"
#include "mcc/misc/NetStatistics.h"
#include "mcc/messages/Deaclarations.h"
#include "mcc/encoder/core/Exchanger.h"


namespace mcc { namespace encoder { namespace core { class Channel; } } }

namespace mcc {
namespace encoder {
namespace core {

class DeviceTask;

class Device
{
protected:
    Device(const mcc::misc::ProtocolId& id, const std::string& device_name, const mcc::messages::MessageSender& out);

public:
    virtual ~Device();

    std::string name() const { return _device_name; }
    std::string firmware() const { return _firmware_name; }
    bool isActive() const { return _isActive; }
    bool isRegistered() const { return _isRegistered; }
    mcc::messages::StatDevice getStatistics() const;
    bool isSame(const mcc::misc::ProtocolId& params) const;
    const mcc::misc::ProtocolId& protocolId() const { return _protocolId; }
    std::size_t id() const { return _protocolId.id(); }

    bmcl::Option<std::size_t>  request(std::vector<uint8_t>* packet);
    virtual void syncAccept(const std::vector<uint8_t>& packet, std::size_t client);
    virtual void asyncAccept(const std::vector<uint8_t>& packet);

    virtual void acceptFirmwareResponse(const std::string& name, const mcc::misc::FirmwareDescriptionPtr& description, const std::string& error) { assert(false); /*not implemented*/ }
    virtual void acceptDeviceUpdate(const std::string& err) {}
    virtual void updateFirmware(const mcc::misc::FirmwareDescriptionPtr& firmware);
    virtual void activate(bool isActive) = 0;
    virtual float regState() const = 0;

    virtual void fileLoad(std::unique_ptr<mcc::messages::DeviceFileLoad_Request>&& request);
    virtual void fileLoadCancel(std::unique_ptr<mcc::messages::DeviceFileLoadCancel_Request>&& request);
    virtual mcc::messages::StatFiles fileLoadState() const { return mcc::messages::StatFiles(); };

    virtual void pushCmd(std::unique_ptr<mcc::messages::Cmd>&& cmd) = 0;
    virtual void cancelCmd(std::unique_ptr<mcc::messages::CmdCancel>&& cancel) = 0;
    virtual std::size_t cmdsInQueue() const = 0;
    virtual std::size_t cmdsProcessed() const = 0;

protected:
    void activate_(bool state);

    void addTask_(const std::shared_ptr<DeviceTask>& task)
    {
        _tasks.push_back(task);
    }

    void registrate_()
    {
        _isRegistered = true;
    }

    mcc::messages::MessageSender _out;
    mutable std::mutex   _mutex;
    mutable mcc::messages::StatDevice _stats;

private:
    mcc::misc::ProtocolId        _protocolId;
    std::string _device_name;
    std::string _firmware_name;

    std::atomic<bool>    _isActive;
    std::atomic<bool>    _isRegistered;

    std::size_t _client = 0;
    std::vector<std::shared_ptr<DeviceTask>>  _tasks;
};

}
}
}
