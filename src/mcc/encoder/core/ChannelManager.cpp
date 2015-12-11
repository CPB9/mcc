/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include <QDebug>
#include <chrono>

#include "bmcl/MemReader.h"

#include "mcc/messages/MessageSender.h"
#include "mcc/messages/Device.h"
#include "mcc/messages/Channel.h"
#include "mcc/encoder/core/Channel.h"
#include "mcc/encoder/core/ChannelManager.h"
#include "mcc/encoder/core/Exchanger.h"
#include "mcc/encoder/core/Device.h"


namespace mcc {
namespace encoder {
namespace core {

ChannelManager::ChannelManager(bool syncExchange
                            , std::unique_ptr<mcc::messages::Channel_Request>&& request
                            , const mcc::messages::MessageSender& out
                            , const mcc::encoder::core::PacketSearcher& searcher
                            , const ChannelOpenerPtr& channelOpener
                            , std::size_t syncResponseTimeoutMs
                            , std::size_t syncNoExchangePauseMs
                            , std::size_t asyncSleepOnReceiveMs
                            )
                            : Runnable(request->address())
                            , _syncResponseTimeoutMs(syncResponseTimeoutMs)
                            , _syncNoExchangePauseMs(syncNoExchangePauseMs)
                            , _asyncSleepOnReceiveMs(asyncSleepOnReceiveMs)
                            , _syncExchange(syncExchange)
                            , _out(out)
                            , _exchanger(request->address(), request->settings().unwrap(), searcher, channelOpener)
{
    _request = std::move(request);

    _hasRequest = false;
    _isDevicesChanged = false;
    _stats.reset();
    _stats._address = _exchanger.address();
    _stats._isConnected = false;
}

ChannelManager::~ChannelManager()
{
    finish(true);
}

void ChannelManager::addDevice(const std::shared_ptr<Device>& device)
{
    std::lock_guard<std::mutex> lock(_mutex);
    auto i = std::find_if(_devicesTemp.begin(), _devicesTemp.end(), [&device](const std::shared_ptr<Device>& d){return d->name() == device->name(); });
    if (i != _devicesTemp.end())
        return;

    _devicesTemp.push_back(device);
    _isDevicesChanged = true;
    qDebug() << "device " << QString::fromStdString(device->name()) << "connected to" << _exchanger.address().c_str();
    _out->send<mcc::messages::DeviceActionLog>("device", device->name(), "connected", _exchanger.address());
    _wakeup.notify_one();
}

void ChannelManager::removeDevice(const std::string& name)
{
    std::lock_guard<std::mutex> lock(_mutex);

    auto dev = std::find_if(_devicesTemp.begin(), _devicesTemp.end(), [&name](const std::shared_ptr<Device>& device) { return device->name() == name; });
    if (dev == _devicesTemp.end())
        return;

    _devicesTemp.erase(dev);
    qDebug() << "device " << QString::fromStdString(name) << "disconnected from" << _exchanger.address().c_str();
    _out->send<mcc::messages::DeviceActionLog>("device", name, "disconnected", _exchanger.address());
    _isDevicesChanged = true;
    _wakeup.notify_one();
}

mcc::messages::StatChannel ChannelManager::getStatistics() const
{
    std::lock_guard<std::mutex> lock(_mutex);
    return _stats;
}

bool ChannelManager::isOpen() const
{
    std::lock_guard<std::mutex> lock(_mutex);
    return (_exchanger.isOpen());
}

void ChannelManager::request(std::unique_ptr<mcc::messages::Channel_Request>&& request)
{
    std::lock_guard<std::mutex> lock(_mutex);
    _request = std::move(request);
    _hasRequest = true;
    _wakeup.notify_one();
}

bool ChannelManager::pre()
{
    std::lock_guard<std::mutex> lock(_mutex);
    executeRequest_();
    return true;
}

void ChannelManager::post()
{
    _exchanger.stop();
}

void ChannelManager::tick()
{
    if (_hasRequest || _isDevicesChanged)
    {
        std::lock_guard<std::mutex> lock(_mutex);
        if (_hasRequest)
        {
            executeRequest_();
            _hasRequest = false;
        }
        if (_isDevicesChanged)
        {
            _devices = _devicesTemp;
            _stats._devices.clear();
            _stats._devices.reserve(_devicesTemp.size());
            for (const auto& i : _devicesTemp)
            {
                _stats._devices.emplace_back(i->name());
            }
            _isDevicesChanged = false;
        }
    }

    if (isSleepCondition_(_devices))
    {
        std::unique_lock<std::mutex> lock(_mutex);
        if (isSleepCondition_(_devices))
            _wakeup.wait(lock, [this](){return !isSleepCondition_(_devices); });
    }

    if (!_exchanger.isOpen())
        return;

    if (_syncExchange)
        syncExchange_(_devices);
    else
        asyncExchange_(_devices);

    {
        std::lock_guard<std::mutex> lock(_mutex);
        _stats._rcvd = _exchanger.statRcvd();
        _stats._sent = _exchanger.statSent();
        _stats._bad  = _exchanger.statBad();
    }
}

void ChannelManager::syncExchange_(const std::vector<std::shared_ptr<Device>>& devices)
{
    bool gotExchange = false;

    for (const auto& device : devices)
    {
        auto client = device->request(&_bufferIn);
        if (!_bufferIn.empty() && client.isSome())
        {
            gotExchange = true;
            _exchanger.clear();
            _exchanger.sendData(_bufferIn);
            _exchanger.receivePacket(_syncResponseTimeoutMs, &_bufferOut);
            device->syncAccept(_bufferOut, client.unwrap());
        }
    }

    //std::this_thread::sleep_for(std::chrono::milliseconds(10));
    if (!gotExchange)
        std::this_thread::sleep_for(std::chrono::milliseconds(_syncNoExchangePauseMs));
}

void ChannelManager::asyncExchange_(const std::vector<std::shared_ptr<Device>>& devices)
{
    for (const auto& device : devices)
    {
        device->request(&_bufferIn);
        if (!_bufferIn.empty())
            _exchanger.sendData(_bufferIn);
    }

    _exchanger.receiveData(_asyncSleepOnReceiveMs);
    while (true)
    {
        auto r = _exchanger.getPacket(&_bufferOut);
        if (r.isNone())
        {
            break;
        }
        PacketDetails p = r.take();
        if (p._deviceId.isNone())
        {
            std::for_each(devices.begin(), devices.end(), [this](const std::shared_ptr<Device>& device){ device->asyncAccept(_bufferOut); });
        }
        else
        {
            auto receiver = p._deviceId.take();
            auto i = std::find_if(devices.begin(), devices.end(), [receiver](const std::shared_ptr<Device>& device){return device->id() == receiver; });
            if (i != devices.end())
                (*i)->asyncAccept(_bufferOut);
        }
    }
}

bool ChannelManager::isSleepCondition_(const std::vector<std::shared_ptr<Device>>& devices) const
{
    if (!isRunning_() || _hasRequest || _isDevicesChanged)
        return false;

    if (!_exchanger.isOpen())
        return true;

    return devices.empty();
}

void ChannelManager::executeRequest_()
{
    std::string text;
    std::string err;
    switch (_request->operation())
    {
    case mcc::messages::Open:
    {
        err = _exchanger.open();
        text = "opened";
        break;
    }
    case mcc::messages::Close:
    {
        err = _exchanger.close();
        text = "closed";
        break;
    }
    case mcc::messages::Create:
    {
        err = _exchanger.start();
        text = "created";
        break;
    }
    case mcc::messages::Remove:
    {
        err = _exchanger.stop();
        stopRunning_();
        text = "stoped";
        break;
    }
    }

    _stats._isConnected = _exchanger.isOpen();

    if (_request)
    {
        _out->respond<mcc::messages::Channel_Response>(_request.get(), err);
        _request.reset(nullptr);
    }

    _out->send<mcc::messages::DeviceActionLog>("channel", _exchanger.address(), text);
    qDebug() << "channel" << QString::fromStdString(text);
}


}
}
}
