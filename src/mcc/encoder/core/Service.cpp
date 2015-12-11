/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include <set>
#include <QDebug>
#include <QCoreApplication>
#include "bmcl/Result.h"

#include "mcc/Names.h"
#include "mcc/messages/MessageSender.h"
#include "mcc/misc/Crc.h"
#include "mcc/encoder/core/Service.h"
#include "mcc/encoder/core/ChannelManager.h"
#include "mcc/encoder/core/Device.h"

#include "mcc/messages/Channel.h"
#include "mcc/messages/Cmd.h"
#include "mcc/messages/Device.h"
#include "mcc/messages/System.h"
#include "mcc/messages/Firmware.h"
#include "mcc/messages/Protocol.h"


namespace mcc {
namespace encoder {
namespace core {

Service::Service(const std::string& name, const mcc::messages::LocalRouterPtr& router, const std::string& protocolName, const PacketSearcher& packetSearcher, bool syncExchange, const ChannelOpenerPtr& channelOpener)
    : mcc::messages::ServiceAbstract(name, router), _syncExchange(syncExchange), _protocolName(protocolName), _packetSearcher(packetSearcher), _channelOpener(channelOpener)
{
}

Service::~Service()
{
    assert(!isRunning_());
}

bool Service::pre()
{
    if (!ServiceAbstract::pre())
        return false;
    if (!waitSystemStarted_())
        return false;
    _out->send<mcc::messages::DeviceList_Request>();
    return true;
}

void Service::post()
{
    for (auto& i : _channels)
    {
        i.second->finish(true);
    }

    _channels.clear();
    _devices.clear();
}

void Service::tick()
{
    auto m = _in->tryRecvFor(std::chrono::milliseconds(100));
    if (m.isOk())
        chooseProcessor(m.take());

    for (auto iter = _channels.begin(); iter != _channels.end();)
    {
        if (iter->second->isFinished())
            _channels.erase(iter++);
        else
            ++iter;
    }
}

void Service::process(std::unique_ptr<mcc::messages::Cmd>&& cmd)
{
    auto device = _devices.find(cmd->device());
    if (device == _devices.end())
    {
        _out->send<mcc::messages::CmdState>(*cmd, mcc::messages::CmdState::Value::Failed, "unknown device");
        return;
    }

    if (!device->second->isActive())
    {
        _out->send<mcc::messages::CmdState>(*cmd, mcc::messages::CmdState::Value::Failed, "device is not active");
        return;
    }

    _out->send<mcc::messages::CmdState>(*cmd, mcc::messages::CmdState::Value::RoutedForDelivery);
    device->second->pushCmd(std::move(cmd));
}

void Service::process(std::unique_ptr<mcc::messages::CmdCancel>&& cancel)
{
    auto i = _devices.find(cancel->device());
    if (i == _devices.end())
        return;

    i->second->cancelCmd(std::move(cancel));
}

void Service::process(std::unique_ptr<mcc::messages::SystemComponentState_Request>&& request)
{
    std::set<std::string> active_devices;
    {
        mcc::messages::StatChannels cs;
        cs.reserve(_channels.size());
        for (const auto& i : _channels)
        {
            cs.emplace_back(i.second->getStatistics());
            for (const auto& d : cs.back()._devices)
                active_devices.insert(d);
        }
        _out->sendTo<mcc::messages::ChannelState_Response>(request->sender(), std::move(cs));
    }

    {
        mcc::messages::StatDevices ds;
        ds.reserve(_devices.size());
        for (const auto& i : _devices)
        {
            if (active_devices.find(i.first) != active_devices.end())
                ds.emplace_back(i.second->getStatistics());
        }
        _out->sendTo<mcc::messages::DeviceState_Response>(request->sender(), std::move(ds));
    }
}

void Service::processChannelCreate(std::unique_ptr<mcc::messages::Channel_Request>&& request)
{
    auto r = mcc::misc::NetChannel::deserialize(QString::fromStdString(request->settings().unwrap()));
    if (r.isNone())
    {
        _out->respond<mcc::messages::Channel_Response>(request.get(), "invalid channel settings" + request->settings().unwrap());
        return;
    }

    if (r->protocol().toStdString() != protocol())
    {
        _out->respond<mcc::messages::Channel_Response>(request.get(), "protocol service incorrect: " + r->protocol().toStdString() + " instead of " + protocol());
        return;
    }

    const auto& i = _channels.find(request->address());
    if (i != _channels.end())
    {
        _out->respond<mcc::messages::Channel_Response>(request.get(), "channel already exists");
        return;
    }

    auto p = mcc::misc::Runnable::startInThread<mcc::encoder::core::ChannelManager>(_syncExchange, std::move(request), _out, _packetSearcher, _channelOpener);
    std::string name = p->name();
    _channels.emplace(name, std::move(p));
}

void Service::processChannelRemove(std::unique_ptr<mcc::messages::Channel_Request>&& request)
{
    const auto& i = _channels.find(request->address());
    if (i == _channels.end())
    {
        _out->respond<mcc::messages::Channel_Response>(request.get());
        return;
    }

    i->second->request(std::move(request));
}

void Service::processChannelOpen(std::unique_ptr<mcc::messages::Channel_Request>&& request)
{
    const auto& i = _channels.find(request->address());
    if (i == _channels.end())
    {
        _out->respond<mcc::messages::Channel_Response>(request.get(), "create channel first");
        return;
    }

    i->second->request(std::move(request));
}

void Service::processChannelClose(std::unique_ptr<mcc::messages::Channel_Request>&& request)
{
    const auto& i = _channels.find(request->address());
    if (i == _channels.end())
    {
        _out->respond<mcc::messages::Channel_Response>(request.get());
        return;
    }

    i->second->request(std::move(request));
}

void Service::process(std::unique_ptr<mcc::messages::Channel_Request>&& request)
{
    switch (request->operation())
    {
    case mcc::messages::Create: processChannelCreate(std::move(request)); break;
    case mcc::messages::Remove: processChannelRemove(std::move(request)); break;
    case mcc::messages::Open: processChannelOpen(std::move(request)); break;
    case mcc::messages::Close: processChannelClose(std::move(request)); break;
    }
}

void Service::process(std::unique_ptr<mcc::messages::DeviceActivate_Request>&& request)
{
    auto d = _devices.find(request->device());
    if (d == _devices.end())
    {
        _out->respond<mcc::messages::DeviceActivate_Response>(request.get(), "unknown device");
        return;
    }

    d->second->activate(request->isActive());
    _out->respond<mcc::messages::DeviceActivate_Response>(request.get());
}

void Service::process(std::unique_ptr<mcc::messages::DeviceConnect_Request>&& request)
{
    auto c = _channels.find(request->channel());
    if (c == _channels.end())
    {
        _out->respond<mcc::messages::DeviceConnect_Response>(request.get(), "open connection first for channel " + request->channel());
        return;
    }

    auto d = _devices.find(request->device());
    if (d != _devices.end())
    {
        c->second->addDevice(d->second);
        _out->send<mcc::messages::CmdSubscribe_Request>(true, request->device());
        _out->respond<mcc::messages::DeviceConnect_Response>(request.get());
        return;
    }

    _out->send<mcc::messages::ProtocolForDevice_Request>(request->device()); //на случай, если параметры соединения изменились
    std::string name = request->device();
    _connectRequests.emplace(name, std::move(request));
}

void Service::process(std::unique_ptr<mcc::messages::DeviceDisconnect_Request>&& request)
{
    auto c = _channels.find(request->channel());
    if (c == _channels.end())
    {
        _out->respond<mcc::messages::DeviceDisconnect_Response>(request.get(), "connection not found");
        return;
    }
    c->second->removeDevice(request->device());

    auto d = _devices.find(request->device());
    if (d == _devices.end())
    {
        _out->respond<mcc::messages::DeviceDisconnect_Response>(request.get(), "unknown device");
        return;
    }

    if (!d->second->isRegistered())
        _devices.erase(d);

    _out->respond<mcc::messages::DeviceDisconnect_Response>(request.get());
}

void Service::process(std::unique_ptr<mcc::messages::DeviceList_Response>&& list)
{
    for (const auto& i: list->devices())
    {
        _out->send<mcc::messages::DeviceDescription_Request>(i);
    }
}

void Service::process(std::unique_ptr<mcc::messages::DeviceDescription_Response>&& description)
{
    const auto i = _devices.find(description->device());
    if (i != _devices.end())
    {
        //уже зарегистрирован - merge?
        return;
    }
}

void Service::process(std::unique_ptr<mcc::messages::DeviceFileLoad_Request>&& request)
{
    auto i = _devices.find(request->device());
    if (i == _devices.end())
    {
        _out->respond<mcc::messages::DeviceFileLoad_Response>(request.get(), "unknown device");
        return;
    }

    i->second->fileLoad(std::move(request));
}

void Service::process(std::unique_ptr<mcc::messages::DeviceFileLoadCancel_Request>&& request)
{
    auto i = _devices.find(request->device());
    if (i == _devices.end())
    {
        _out->respond<mcc::messages::DeviceFileLoad_Response>(request.get(), "unknown device");
        return;
    }

    i->second->fileLoadCancel(std::move(request));
}

void Service::process(std::unique_ptr<mcc::messages::ProtocolForDevice_Response>&& response)
{
    const std::string& device = response->device();

    const auto& ids = response->protocols();
    const std::string& p = protocol();
    const auto& id = std::find_if(ids.begin(), ids.end(), [&p](const mcc::misc::ProtocolId& id){ return id.protocol().toStdString() == p; });
    if (id == ids.end())
    {
        const auto& r = _connectRequests.find(response->device());
        if (r != _connectRequests.end())
        {
            _out->respond<mcc::messages::DeviceConnect_Response>(r->second.get(), "Protocol " + protocol() + " id for device " + device + " is not set");
            _connectRequests.erase(r);
        }

        const auto d = _devices.find(device);
        if (d != _devices.end())
        {   /*идентификатор аппарата по данному протоколу не задан -> удалить аппарат*/
            assert(false);
            _devices.erase(d);
        }

        return;
    }

    const auto& r = _connectRequests.find(response->device());
    auto d = _devices.find(device);
    if (d != _devices.end())
    {
        if (!d->second->isSame(*id))
        {
            /*идентификация аппарата изменилась -> изменить везде!*/
            assert(false);
        }
    }
    else
    {
        if (r != _connectRequests.end())
        {
            _devices.emplace(device, createDevice(*id, response->device()));
            d = _devices.find(device);
        }
    }

    if (r != _connectRequests.end())
    {
        auto c = _channels.find(r->second->channel());
        if (c == _channels.end())
        {
            _out->respond<mcc::messages::DeviceConnect_Response>(r->second.get(), "channel not found");
            _connectRequests.erase(r);
            return;
        }

        c->second->addDevice(d->second);
        _out->send<mcc::messages::CmdSubscribe_Request>(true, device);
        _out->respond<mcc::messages::DeviceConnect_Response>(r->second.get());
        _connectRequests.erase(r);
    }
}

void Service::process(std::unique_ptr<mcc::messages::FirmwareDescription_Response>&& response)
{
    for (auto& device : _devices)
    {
        device.second->acceptFirmwareResponse(response->firmware(), response->description(), response->error());
    }
}

void Service::process(std::unique_ptr<mcc::messages::FirmwareRegister_Response>&& response)
{
    const auto& i = _devices.find(response->source());
    if (i == _devices.end())
        return;
    i->second->acceptFirmwareResponse(response->name(), nullptr, response->error());
}

void Service::process(std::unique_ptr<mcc::messages::DeviceUnRegistered>&& notify)
{
    for (const auto& i : _channels)
    {
        i.second->removeDevice(notify->device());
    }

    _devices.erase(notify->device());
}

void Service::process(std::unique_ptr<mcc::messages::DeviceUpdate_Response>&& response)
{
    const auto& i = _devices.find(response->device());
    if (i == _devices.end())
        return;
    i->second->acceptDeviceUpdate(response->error());
}

void Service::process(std::unique_ptr<mcc::messages::DeviceUpdated>&& response)
{
    auto i = _devices.find(response->device());
    if (i == _devices.end())
        return;

    //i->second->updateFirmware(response->firmware());
}

}
}
}
