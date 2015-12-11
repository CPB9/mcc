/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include <QMetaType>
#include <QCoreApplication>
#include <QDebug>

#include "mcc/ui/exchange/Service.h"
#include "mcc/Names.h"
#include "mcc/misc/TimeUtils.h"
#include "mcc/messages/MessageSender.h"
#include "mcc/messages/Channel.h"
#include "mcc/messages/Cmd.h"
#include "mcc/messages/Device.h"
#include "mcc/messages/System.h"
#include "mcc/messages/Tm.h"
#include "mcc/messages/Protocol.h"
#include "mcc/messages/Firmware.h"


namespace mcc {
namespace ui {
namespace exchange {

Service::Service(const mcc::messages::LocalRouterPtr& router)
    : mcc::messages::MessageProcessor(router->send(mcc::Names::ui()))
    , _name(mcc::Names::ui())
    , _systemStarted(false)
    , _in(router->recv(mcc::Names::ui()))
    , _out(router->send(mcc::Names::ui()))
{
    qRegisterMetaType<mcc::misc::TmParam>();
    qRegisterMetaType<mcc::misc::Cmd>();
    qRegisterMetaType<mcc::misc::CmdCollationId>();
    qRegisterMetaType<mcc::misc::CmdState>();
    qRegisterMetaType<mcc::misc::NetChannel>();
    qRegisterMetaType<mcc::misc::NetState>();
    qRegisterMetaType<mcc::misc::NetStateList>();
    qRegisterMetaType<mcc::misc::ConnectionState>();
    qRegisterMetaType<mcc::misc::DeviceState>();
    qRegisterMetaType<mcc::misc::DeviceStateList>();
    qRegisterMetaType<QVector<QString>>();
    qRegisterMetaType<mcc::misc::DeviceDescription>();
    qRegisterMetaType<mcc::misc::ProtocolDescription>();
    qRegisterMetaType<mcc::misc::ProtocolId>();
    qRegisterMetaType<QVector<mcc::misc::ProtocolId>>();

    qRegisterMetaType<mcc::misc::CmdParamDescription>();
    qRegisterMetaType<mcc::misc::CmdDescription>();
    qRegisterMetaType<mcc::misc::TmParamDescription>();
    qRegisterMetaType<mcc::misc::TraitDescription>();
    qRegisterMetaType<mcc::misc::TraitDescriptionList>();
    qRegisterMetaType<mcc::misc::FirmwareDescription>();

    _timer = startTimer(100);
}

Service::~Service()
{
    stop();
}

void Service::stop()
{
    _in->close();
    QObject::killTimer(_timer);
}

void Service::pre()
{
    _out->send<mcc::messages::ProtocolList_Request>();
}

void Service::process(std::unique_ptr<mcc::messages::TmParamSubscribe_Response>&& response)
{
    emit tmParamSubscribeResponse(mcc::misc::TmParam(QString::fromStdString(response->device()), QString::fromStdString(response->trait()), QString::fromStdString(response->status())));
}

void Service::process(std::unique_ptr<mcc::messages::TmParamList>&& response)
{
    if (response->params().empty())
        return;

    std::map<QString, mcc::misc::TmParam> params;
    QString device = QString::fromStdString(response->device());
    for (const auto& i : response->params())
    {
        QString trait = QString::fromStdString(i.trait());
        QString name = QString::fromStdString(i.status());
        QString full_name = QString("%1.%2").arg(trait).arg(name);
        mcc::misc::TmParam param(device, std::move(trait), std::move(name), std::move(i.value()));
        params.emplace(std::move(full_name), std::move(param));
    }
    emit tmParamList(QDateTime::fromString(QString::fromStdString(response->time()), mcc::misc::dateSerializeFormat()), params);
}

void Service::process(std::unique_ptr<mcc::messages::CmdState>&& state)
{
    QString err = QString::fromStdString(state->reason());
    QStringList response;
//     for (const auto& i : body.response())
//     {
//         response.push_back(QString::number(*(uint8_t*)i));
//     }
    emit cmdState(QDateTime::fromString(QString::fromStdString(state->time()), mcc::misc::dateSerializeFormat()), state->cmdId(), (mcc::misc::CmdState)state->state(), response, err);
}

void Service::process(std::unique_ptr<mcc::messages::Channel_Response>&& response)
{
    const mcc::misc::NetChannel& channel = _nameToChannel[response->address()];
    QString error = QString::fromStdString(response->error());

    switch (response->operation())
    {
    case mcc::messages::Create:
        if (response->error().empty())
            emit channelCreated(channel);
        else
            emit channelCreateFailure(channel, error);
        break;
    case mcc::messages::Remove:
        emit channelRemoved(channel);
        break;
    case mcc::messages::Open:
        if (response->error().empty())
            emit channelOpened(channel);
        else
            emit channelOpenFailure(channel, error);
        break;
    case mcc::messages::Close:
        emit channelClosed(channel);
        break;
    }
}

void Service::process(std::unique_ptr<mcc::messages::ChannelState_Response>&& response)
{
    mcc::misc::NetStateList list;
    QString service = QString::fromStdString(response->sender());

    for (const auto& i : response->state())
    {
        const mcc::misc::NetChannel& channel = _nameToChannel[i._address];
        mcc::misc::NetState state(channel.serialize(), i._isConnected ? mcc::misc::ConnectionState::connected : mcc::misc::ConnectionState::disconnected);
        for (const auto& j : i._devices)
        {
            state._devices.push_back(QString::fromStdString(j));
        }

        state._stats._sentBytes = i._sent._bytes;
        state._stats._sentPackets = i._sent._packets;
        state._stats._sent = QDateTime::fromString(QString::fromStdString(i._sent._time), mcc::misc::dateSerializeFormat());
        state._stats._rcvdBytes = i._rcvd._bytes;
        state._stats._rcvdPackets = i._rcvd._packets;
        state._stats._rcvd = QDateTime::fromString(QString::fromStdString(i._rcvd._time), mcc::misc::dateSerializeFormat());
        state._stats._rcvdBadBytes = i._bad._bytes;
        state._stats._rcvdBadPackets = i._bad._packets;
        state._stats._rcvdBad = QDateTime::fromString(QString::fromStdString(i._bad._time), mcc::misc::dateSerializeFormat());

        list.push_back(state);
    }

    emit channelState(service, list);
}

void Service::process(std::unique_ptr<mcc::messages::DeviceState_Response>&& response)
{
    mcc::misc::DeviceStateList list;
    QString service = QString::fromStdString(response->sender());

    for (const auto& i : response->state())
    {
        QString device = QString::fromStdString(i._device);
        mcc::misc::DeviceState state(device, i._isActive, i._isRegistered, i._regState);

        state._stats._sentBytes = i._sent._bytes;
        state._stats._sentPackets = i._sent._packets;
        state._stats._sent = QDateTime::fromString(QString::fromStdString(i._sent._time), mcc::misc::dateSerializeFormat());
        state._stats._rcvdBytes = i._rcvd._bytes;
        state._stats._rcvdPackets = i._rcvd._packets;
        state._stats._rcvd = QDateTime::fromString(QString::fromStdString(i._rcvd._time), mcc::misc::dateSerializeFormat());
        state._stats._rcvdBadBytes = i._bad._bytes;
        state._stats._rcvdBadPackets = i._bad._packets;
        state._stats._rcvdBad = QDateTime::fromString(QString::fromStdString(i._bad._time), mcc::misc::dateSerializeFormat());

        for (const auto& j : i._files)
        {
            state._files.emplace_back(j._file_path, j._fileSize, j._bytesLoaded, j._isUp ? mcc::misc::FileTransfer::Direction::Up : mcc::misc::FileTransfer::Direction::Down);
        }

        list.push_back(state);
    }

    emit deviceState(service, list);
}

void Service::process(std::unique_ptr<mcc::messages::DeviceActivate_Response>&& response)
{
    QString device = QString::fromStdString(response->device());
    QString error = QString::fromStdString(response->error());

    if (error.isEmpty())
        emit deviceActivated(device, response->isActive());
    else
        emit deviceActivationFailure(device, error);
}

void Service::process(std::unique_ptr<mcc::messages::DeviceConnect_Response>&& response)
{
    QString device = QString::fromStdString(response->device());
    const mcc::misc::NetChannel& channel = _nameToChannel[response->channel()];
    QString error = QString::fromStdString(response->error());

    if (error.isEmpty())
        emit deviceConnected(channel, device);
    else
        emit deviceConnectFailed(channel, device, error);
}

void Service::process(std::unique_ptr<mcc::messages::DeviceDisconnect_Response>&& response)
{
    QString device = QString::fromStdString(response->device());
    QString error = QString::fromStdString(response->error());
    const mcc::misc::NetChannel& channel = _nameToChannel[response->channel()];

    emit deviceDisconnected(channel, device, error);
}

void Service::process(std::unique_ptr<mcc::messages::DeviceRegister_Response>&& response)
{
    if (response->error().empty())
        emit deviceRegistered(response->id(), QString::fromStdString(response->name()), QString::fromStdString(response->info()));
    else
        emit deviceRegisterFailure(QString::fromStdString(response->info()), QString::fromStdString(response->error()));
}

void Service::process(std::unique_ptr<mcc::messages::DeviceList_Response>&& response)
{
    QVector<QString> devices;
    devices.reserve(response->devices().size());
    for (const auto& device : response->devices())
    {
        devices.push_back(QString::fromStdString(device));
    }

    emit deviceList(devices);
}

void Service::process(std::unique_ptr<mcc::messages::DeviceRegistered>&& response)
{
    emit deviceRegistered(response->id(), QString::fromStdString(response->name()), QString::fromStdString(response->info()));
}

void Service::process(std::unique_ptr<mcc::messages::DeviceUnRegistered>&& response)
{
    emit deviceUnRegistered(QString::fromStdString(response->device()));
}

void Service::process(std::unique_ptr<mcc::messages::DeviceUpdated>&& notify)
{
    emit deviceUpdated(QString::fromStdString(notify->device()));
}

void Service::process(std::unique_ptr<mcc::messages::DeviceUnRegister_Response>&& response)
{
    emit deviceUnRegistered(QString::fromStdString(response->device()));
}

void Service::process(std::unique_ptr<mcc::messages::DeviceDescription_Response>&& response)
{
    if (!response->error().empty())
    {
        emit deviceDescriptionFailure(QString::fromStdString(response->device()), QString::fromStdString(response->error()));
        return;
    }

    mcc::misc::DeviceDescription d;
    d._device_id = response->device_id();
    d._device_name = QString::fromStdString(response->device());
    d._device_info = QString::fromStdString(response->device_info());

    d._kind_id = response->kind_id();
    d._kind_name = QString::fromStdString(response->kind_name());
    d._kind_info = QString::fromStdString(response->kind_info());

    d._firmware_id = response->firmware_id();
    d._firmware_name = QString::fromStdString(response->firmware_name());
    d._firmware_info = QString::fromStdString(response->firmware_info());

    emit deviceDescription(d);
}

void Service::process(std::unique_ptr<mcc::messages::DeviceFileLoad_Response>&& response)
{
    if (response->error().empty())
        emit deviceFileLoaded(QString::fromStdString(response->device()), QString::fromStdString(response->file_path()));
    else
        emit deviceFileLoadFailed(QString::fromStdString(response->device()), QString::fromStdString(response->file_path()), QString::fromStdString(response->error()));
}

void Service::process(std::unique_ptr<mcc::messages::FirmwareDescription_Response>&& response)
{
    if (!response->error().empty())
    {
        emit firmwareDescriptionFailure(QString::fromStdString(response->firmware()), QString::fromStdString(response->error()));
        return;
    }

    emit firmwareDescription(QString::fromStdString(response->firmware()), response->description()->_traits);
}

void Service::process(std::unique_ptr<mcc::messages::FirmwareRegistered>&& notification)
{
    if (notification->isRegistered())
        emit FirmwareRegistered(notification->id(), QString::fromStdString(notification->name()));
    else
        emit FirmwareUnRegistered(notification->id(), QString::fromStdString(notification->name()));
}

void Service::process(std::unique_ptr<mcc::messages::ProtocolList_Response>&& response)
{
    QVector<QString> protocols;
    protocols.reserve(response->protocols().size());
    for (const auto& p : response->protocols())
    {
        protocols.push_back(QString::fromStdString(p));
        _out->send<mcc::messages::ProtocolDescription_Request>(p);
    }

    emit protocolList(protocols);
}

void Service::process(std::unique_ptr<mcc::messages::ProtocolDescription_Response>&& response)
{
    if (!response->error().empty())
    {
        emit protocolDescriptionFailed(QString::fromStdString(response->protocol()), QString::fromStdString(response->error()));
        return;
    }
    _protocolToExchange.emplace(response->protocol(), response->description()._service);

    mcc::misc::ProtocolDescription description;
    description.id = response->description()._id;
    description.info = QString::fromStdString(response->description()._info);
    description.param_info = QString::fromStdString(response->description()._param_info);
    description.name = QString::fromStdString(response->description()._name);
    description.service = QString::fromStdString(response->description()._service);
    description.trait = QString::fromStdString(response->description()._trait);

    emit protocolDescription(description);
}

void Service::process(std::unique_ptr<mcc::messages::ProtocolForDevice_Response>&& response)
{
    const auto& ids = response->protocols();
    QVector<mcc::misc::ProtocolId> r;
    for (const auto& i : ids)
    {
        r.push_back(i);
    }
    emit protocolForDevice(QString::fromStdString(response->device()), r);
}

void Service::process(std::unique_ptr<mcc::messages::ProtocolForDeviceRegister_Response>&& response)
{
    if (response->error().empty())
        emit protocolForDeviceRegistered(QString::fromStdString(response->device()), response->protocol());
    else
        emit protocolForDeviceRegisterFailed(QString::fromStdString(response->device()), response->protocol(), QString::fromStdString(response->error()));
}

void Service::process(std::unique_ptr<mcc::messages::SystemState>&& response)
{
    _systemStarted = response->isStarted();
    if (_systemStarted)
    {
        pre();
        emit systemStarted();
    }
    else
    {
        emit systemStoped(QString::fromStdString(response->reason()));
    }
}

void Service::timerEvent(QTimerEvent* event)
{
    std::size_t counter = 0;
    while (!_in->isEmpty() && counter < 100)
    {
        auto m = _in->tryRecv();
        if (m.isErr())
            break;

        chooseProcessor(std::move(m.take()));
        ++counter;
    }
}

void Service::onCmd(const mcc::misc::Cmd& cmd)
{
    _out->send<mcc::messages::Cmd>(cmd.collationId(), cmd.device().toStdString(), cmd.trait().toStdString(), cmd.command().toStdString(), cmd.params());
}

void Service::onCmdCancel(const QString& device, mcc::misc::CmdCollationId collationId)
{
    _out->send<mcc::messages::CmdCancel>(device.toStdString(), collationId);
}

void Service::onTmParamSubscribtion(const mcc::misc::TmParam& param)
{
    _out->send<mcc::messages::TmParamSubscribe_Request>(true, param.device().toStdString(), param.trait().toStdString(), param.status().toStdString());
}

void Service::onDeviceDescription(const QString& device)
{
    _out->send<mcc::messages::DeviceDescription_Request>(device.toStdString());
}

void Service::onChannelCreate(const mcc::misc::NetChannel& channel, const QString& service)
{
    _nameToChannel[channel.name().toStdString()] = channel;
    _out->sendTo<mcc::messages::Channel_Request>(service.toStdString(), mcc::messages::Create, channel.name().toStdString(), channel.serialize().toStdString());
}

void Service::onChannelRemove(const mcc::misc::NetChannel& channel, const QString& service)
{
    _out->sendTo<mcc::messages::Channel_Request>(service.toStdString(), mcc::messages::Remove, channel.name().toStdString());
}

void Service::onChannelOpen(const mcc::misc::NetChannel& channel, const QString& service)
{
    _out->sendTo<mcc::messages::Channel_Request>(service.toStdString(), mcc::messages::Open, channel.name().toStdString());
}

void Service::onChannelClose(const mcc::misc::NetChannel& channel, const QString& service)
{
    _out->sendTo<mcc::messages::Channel_Request>(service.toStdString(), mcc::messages::Close, channel.name().toStdString());
}

void Service::onDeviceActivate(const QString& device, bool isActive)
{
    for (const auto& i : _protocolToExchange)
    {
        _out->sendTo<mcc::messages::DeviceActivate_Request>(i.second, device.toStdString(), isActive);
    }
}

void Service::onDeviceConnect(const mcc::misc::NetChannel& channel, const QString& service, const QString& device)
{
    _out->sendTo<mcc::messages::DeviceConnect_Request>(service.toStdString(), device.toStdString(), channel.name().toStdString());
}

void Service::onDeviceDisconnect(const mcc::misc::NetChannel& channel, const QString& service, const QString& device)
{
    _out->sendTo<mcc::messages::DeviceDisconnect_Request>(service.toStdString(), device.toStdString(), channel.name().toStdString());
}

void Service::onDeviceRegister(const QString& info)
{
    _out->send<mcc::messages::DeviceRegister_Request>(info.toStdString());
}

void Service::onDeviceUnRegister(const QString& device)
{
    _out->send<mcc::messages::DeviceUnRegister_Request>(device.toStdString());
}

void Service::onDeviceList()
{
    if (!_systemStarted)
        return;
    _out->send<mcc::messages::DeviceList_Request>();
}

void Service::onDeviceFileLoad(const QString& service, const QString& device, const QString& filePath, mcc::misc::FileTransfer::Direction direction)
{
    if (service.isEmpty())
    {
        emit deviceFileLoadFailed(device, filePath, "service is not set");
        return;
    }
    mcc::messages::DeviceFileLoad_Request::Direction d = (direction == mcc::misc::FileTransfer::Direction::Up ? mcc::messages::DeviceFileLoad_Request::Up : mcc::messages::DeviceFileLoad_Request::Down);
    _out->sendTo<mcc::messages::DeviceFileLoad_Request>(service.toStdString(), device.toStdString(), filePath.toStdString(), d);
}

void Service::onDeviceFileLoadCancel(const QString& service, const QString& device, const QString& filePath, const QString& reason)
{
    _out->sendTo<mcc::messages::DeviceFileLoadCancel_Request>(service.toStdString(), device.toStdString(), filePath.toStdString(), reason.toStdString());
}

void Service::onProtocolList()
{
    if (!_systemStarted)
        return;
    _out->send<mcc::messages::ProtocolList_Request>();
}

void Service::onProtocolDescription(const QString& protocol)
{
    if (!_systemStarted)
        return;
    _out->send<mcc::messages::ProtocolDescription_Request>(protocol.toStdString());
}

void Service::onProtocolForDevice(const QString& device)
{
    _out->send<mcc::messages::ProtocolForDevice_Request>(device.toStdString());
}

void Service::onProtocolForDeviceRegister(const QString& device, const mcc::misc::ProtocolId& id)
{
    _out->send<mcc::messages::ProtocolForDeviceRegister_Request>(device.toStdString(), id);
}

void Service::onFirmwareDescription(const QString& firmware)
{
    if (!_systemStarted)
        return;
    _out->send<mcc::messages::FirmwareDescription_Request>(firmware.toStdString());
}

void Service::onSystemState()
{
    if (_systemStarted)
        emit systemStarted();
    else
        emit systemStoped();
}

void Service::onMccState(const QString& service)
{
    if (!_systemStarted)
        return;
    _out->sendTo<mcc::messages::SystemComponentState_Request>(service.toStdString());
}

}
}
}
