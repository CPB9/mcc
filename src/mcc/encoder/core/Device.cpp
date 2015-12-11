/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include <QDebug>
#include "bmcl/Result.h"

#include "mcc/messages/Device.h"
#include "mcc/messages/MessageSender.h"
#include "mcc/encoder/core/Device.h"
#include "mcc/encoder/core/DeviceTask.h"


namespace mcc {
namespace encoder {
namespace core {

    Device::Device(const mcc::misc::ProtocolId& id, const std::string& device_name, const mcc::messages::MessageSender& out)
        : _out(out), _protocolId(id), _device_name(device_name)
    {
        _isActive = false;
        _isRegistered = false;
        _stats.reset();
        _stats._device = _device_name;
    }

    Device::~Device()
    {
        activate_(false);
        for (const auto& i : _tasks)
        {
            if (!i)
                continue;
            i->_outerChannel.sender.close();
            i->finish(true);
        }
    }

    void Device::activate_(bool state)
    {
        if (_isActive == state)
            return;

        _out->send<mcc::messages::DeviceActionLog>("device", _device_name, state ? "activated" : "deactivated", _protocolId.protocol().toStdString());
        _isActive = state;
        for (auto& i : _tasks)
        {
            i->activate(state);
        }
    }

    mcc::messages::StatDevice Device::getStatistics() const
    {
        std::lock_guard<std::mutex> lock(_mutex);
        _stats._isActive = _isActive;
        _stats._isRegistered = _isRegistered;
        _stats._regState = regState();
        _stats._cmdsInQueue = cmdsInQueue();
        _stats._cmdsProcessed = cmdsProcessed();
        _stats._files = fileLoadState();
        return _stats;
    }

    bool Device::isSame(const mcc::misc::ProtocolId& id) const
    {
        assert(id.protocol() == _protocolId.protocol());
        return _protocolId.id() == id.id();
    }

    void Device::syncAccept(const std::vector<uint8_t>& packet, std::size_t client)
    {
        std::lock_guard<std::mutex> lock(_mutex);
        if (!packet.empty())
            _stats._rcvd.add(packet.size());

        if (client < _tasks.size())
            _tasks[client]->_outerChannel.sender.send(packet);
    }

    void Device::asyncAccept(const std::vector<uint8_t>& packet)
    {
        Q_UNUSED(packet);
        qDebug() << "async accept not implemented";
    }

    bmcl::Option<std::size_t> Device::request(std::vector<uint8_t>* packet)
    {
        packet->clear();
        if (!isActive())
            return bmcl::None;

        std::lock_guard<std::mutex> lock(_mutex);

        std::size_t count = _tasks.size();
        for (std::size_t i = 0; i < count; ++i)
        {
            _client = (_client + 1) % count;
            auto r = _tasks[_client]->_outerChannel.reciever.tryRecv();
            if (r.isOk())
            {
                auto data = r.take();
                _stats._sent.add(data.size());
                packet->swap(data);
                return _client;
            }
        }
        return bmcl::None;
    }

    void Device::fileLoad(std::unique_ptr<mcc::messages::DeviceFileLoad_Request>&& request)
    {
        _out->respond<mcc::messages::DeviceFileLoad_Response>(request.get(), "not implemented");
    }

    void Device::fileLoadCancel(std::unique_ptr<mcc::messages::DeviceFileLoadCancel_Request>&& request)
    {
        _out->respond<mcc::messages::DeviceFileLoad_Response>(request.get(), "not implemented");
    }

    void Device::updateFirmware(const mcc::misc::FirmwareDescriptionPtr& f)
    {
        std::string name = firmware();
        if (name == f->_name.toStdString())
        assert(false); //проверить, что прошивка еще не обновлялась или обновлялась, но другая
        for (const auto& i : _tasks)
        {
            i->updateFirmware(f);
        }
    };


}
}
}
