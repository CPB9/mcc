/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#pragma once
#include <QObject>
#include <QString>
#include <memory>

#include "mcc/misc/TmParam.h"
#include "mcc/misc/Cmd.h"
#include "mcc/misc/Net.h"
#include "mcc/misc/Device.h"
#include "mcc/misc/Firmware.h"
#include "mcc/misc/Protocol.h"
#include "mcc/messages/LocalRouter.h"
#include "mcc/messages/Message.h"


namespace mcc {
namespace ui {
namespace exchange {

class Service: public QObject, public mcc::messages::MessageProcessor
{
    Q_OBJECT
public:
    Service(const mcc::messages::LocalRouterPtr& router);
    virtual ~Service();
    void stop();
    void timerEvent(QTimerEvent * event) override;

public slots:
    void onCmd(const mcc::misc::Cmd& cmd);
    void onCmdCancel(const QString& device, mcc::misc::CmdCollationId collationId);

    void onTmParamSubscribtion(const mcc::misc::TmParam& param);

    void onChannelCreate(const mcc::misc::NetChannel& channel, const QString& service);
    void onChannelRemove(const mcc::misc::NetChannel& channel, const QString& service);
    void onChannelOpen(const mcc::misc::NetChannel& channel, const QString& service);
    void onChannelClose(const mcc::misc::NetChannel& channel, const QString& service);

    void onDeviceActivate(const QString& device, bool isActive);
    void onDeviceConnect(const mcc::misc::NetChannel& channel, const QString& service, const QString& device);
    void onDeviceDisconnect(const mcc::misc::NetChannel& channel, const QString& service, const QString& device);
    void onDeviceRegister(const QString& info);
    void onDeviceUnRegister(const QString& device);
    void onDeviceList();
    void onDeviceDescription(const QString& device);
    void onDeviceFileLoad(const QString& service, const QString& device, const QString& filePath, mcc::misc::FileTransfer::Direction direction);
    void onDeviceFileLoadCancel(const QString& service, const QString& device, const QString& filePath, const QString& reason);

    void onFirmwareDescription(const QString& firmware);

    void onProtocolList();
    void onProtocolDescription(const QString& protocol);
    void onProtocolForDevice(const QString& device);
    void onProtocolForDeviceRegister(const QString& device, const mcc::misc::ProtocolId& id);

    void onSystemState();
    void onMccState(const QString& component);

signals:
    void tmParam(const QDateTime& time, const mcc::misc::TmParam& param);
    void tmParamList(const QDateTime& time, const std::map<QString, mcc::misc::TmParam>& params);
    void tmParamSubscribeResponse(const mcc::misc::TmParam& response);

    void cmdState(const QDateTime& time, mcc::misc::CmdCollationId collationId, mcc::misc::CmdState state, const QStringList& response, const QString& reason = QString());

    void channelCreated(const mcc::misc::NetChannel& channel);
    void channelCreateFailure(const mcc::misc::NetChannel& channel, const QString& reason = QString());
    void channelRemoved(const mcc::misc::NetChannel& channel);

    void channelOpened(const mcc::misc::NetChannel& channel);
    void channelOpenFailure(const mcc::misc::NetChannel& channel, const QString& reason = QString());
    void channelClosed(const mcc::misc::NetChannel& channel);
    void channelState(const QString& encoderService, const mcc::misc::NetStateList& list);

    void deviceActivated(const QString& device, bool isActive);
    void deviceActivationFailure(const QString& device, const QString& reason);
    void deviceConnected(const mcc::misc::NetChannel& channel, const QString& device);
    void deviceConnectFailed(const mcc::misc::NetChannel& channel, const QString& device, const QString& error);
    void deviceDisconnected(const mcc::misc::NetChannel& channel, const QString& device, const QString& error);
    void deviceList(const QVector<QString>& devices);
    void deviceState(const QString& encoderService, const mcc::misc::DeviceStateList& list);
    void deviceRegisterFailure(const QString& info, const QString& error);
    void deviceRegistered(std::size_t id, const QString& name, const QString& info);
    void deviceUnRegistered(const QString& device);
    void deviceUpdated(const QString& device);
    void deviceDescription(const mcc::misc::DeviceDescription& device);
    void deviceDescriptionFailure(const mcc::misc::DeviceDescription& device, const QString& reason);
    void deviceFileLoaded(const QString& device, const QString& filePath);
    void deviceFileLoadFailed(const QString& device, const QString& filePath, const QString& reason);
    void firmwareDescription(const QString& firmware, const mcc::misc::TraitDescriptionList& traits);
    void firmwareDescriptionFailure(const QString& firmware, const QString& error);
    void FirmwareRegistered(std::size_t id, const QString& name);
    void FirmwareUnRegistered(std::size_t id, const QString& name);

    void protocolList(const QVector<QString>& protocols);
    void protocolDescription(const mcc::misc::ProtocolDescription& protocol);
    void protocolDescriptionFailed(const QString& protocol, const QString& error);
    void protocolForDevice(const QString& device, const QVector<mcc::misc::ProtocolId>& ids);
    void protocolForDeviceRegistered(const QString& device, const mcc::misc::ProtocolId& id);
    void protocolForDeviceRegisterFailed(const QString& device, const mcc::misc::ProtocolId& id, const QString& reason);

    void systemStarted();
    void systemStoped(const QString& reason = QString());

private:
    void pre();

    void process(std::unique_ptr<mcc::messages::SystemState>&&) override;
    void process(std::unique_ptr<mcc::messages::CmdState>&&) override;
    void process(std::unique_ptr<mcc::messages::DeviceActivate_Response>&&) override;
    void process(std::unique_ptr<mcc::messages::DeviceConnect_Response>&&) override;
    void process(std::unique_ptr<mcc::messages::DeviceDisconnect_Response>&&) override;
    void process(std::unique_ptr<mcc::messages::DeviceRegister_Response>&&) override;
    void process(std::unique_ptr<mcc::messages::DeviceList_Response>&&) override;
    void process(std::unique_ptr<mcc::messages::DeviceRegistered>&&) override;
    void process(std::unique_ptr<mcc::messages::DeviceUnRegistered>&&) override;
    void process(std::unique_ptr<mcc::messages::DeviceUnRegister_Response>&&) override;
    void process(std::unique_ptr<mcc::messages::DeviceUpdated>&&) override;
    void process(std::unique_ptr<mcc::messages::DeviceDescription_Response>&&) override;
    void process(std::unique_ptr<mcc::messages::DeviceFileLoad_Response>&&) override;
    void process(std::unique_ptr<mcc::messages::DeviceState_Response>&&) override;
    void process(std::unique_ptr<mcc::messages::Channel_Response>&&) override;
    void process(std::unique_ptr<mcc::messages::ChannelState_Response>&&) override;
    void process(std::unique_ptr<mcc::messages::TmParamSubscribe_Response>&& response) override;
    void process(std::unique_ptr<mcc::messages::TmParamList>&& response) override;
    void process(std::unique_ptr<mcc::messages::ProtocolList_Response>&&) override;
    void process(std::unique_ptr<mcc::messages::ProtocolDescription_Response>&&) override;
    void process(std::unique_ptr<mcc::messages::ProtocolForDevice_Response>&&) override;
    void process(std::unique_ptr<mcc::messages::ProtocolForDeviceRegister_Response>&&) override;
    void process(std::unique_ptr<mcc::messages::FirmwareDescription_Response>&&) override;
    void process(std::unique_ptr<mcc::messages::FirmwareRegistered>&&) override;

private:
    int _timer;
    mcc::messages::LocalRouterPtr _router;
    mcc::messages::MessageQueue _in;
    mcc::messages::MessageSender _out;
    std::string _name;
    bool _systemStarted;
    std::map<std::string, std::string> _protocolToExchange;
    std::map<std::string, mcc::misc::NetChannel> _nameToChannel;
};
}
}
}

Q_DECLARE_METATYPE(QVector<QString>);