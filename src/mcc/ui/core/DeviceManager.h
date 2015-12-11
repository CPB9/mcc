/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#pragma once

#include <QObject>
#include <QVector>
#include <QMap>
#include <bmcl/Option.h>

#include "mcc/ui/exchange/Service.h"

#include "mcc/ui/core/FlyingDevice.h"
#include "mcc/ui/core/DevicesGroup.h"

namespace mcc {
    namespace ui {
        namespace core {

            struct ProtocolParamDescriptor
            {
                QString _device;
                QString _trait;
                int     _paramId;

                ProtocolParamDescriptor() :
                    _paramId(-1)
                {

                }

                ProtocolParamDescriptor(const QString& device, const QString& trait, int paramId)
                    : _device(device), _trait(trait), _paramId(paramId)
                {

                }
            };

            class DeviceManager : public QObject
            {
                Q_OBJECT

            public:
                DeviceManager();
                ~DeviceManager();

                FlyingDevice* currentDevice();

                void setPixmapList(const QVector<QPixmap>& pixmaps);

                QVector<QPixmap> pixmapList() const;

                void addDevice(const QString& deviceName, bool setCurrent = false);
                void removeDevice(FlyingDevice* device);

                DevicesGroup* addGroup(const QString& name);
                void removeGroup(DevicesGroup* group);
                std::vector<DevicesGroup*> groups() const { return _devicesGroups; }

                void createFakeDevice();
                void startExchange();
                void stopExchange();

                void clear();

                QVector<FlyingDevice*> devicesList() const;
                mcc::misc::Cmd getSentCmd(mcc::misc::CmdCollationId collationId) const;
                bool readyForExchange(const QString& device) const;
                bmcl::Option<FlyingDevice*> device(const QString& name);
                bmcl::Option<FlyingDevice*> readyForExchangeWithProtocolId(const mcc::misc::ProtocolId& id);
            public slots:
                void processTm(const QDateTime& time, const mcc::misc::TmParam& tmParam);
                void processTmParamList(const QDateTime& time, const std::map<QString, mcc::misc::TmParam>& tmParamList);

                void selectDevice(FlyingDevice* device);
                void sendCmd(const mcc::misc::Cmd& cmd);
                void cancelCmd(const QString& device, mcc::misc::CmdCollationId collationId);

                void onDeviceRegistered(std::size_t id, const QString& name, const QString& info);
                void onDeviceState(const QString& encoderService, const mcc::misc::DeviceStateList & statesList);

                void onDeviceDescription(const mcc::misc::DeviceDescription& device);
                void onFirmwareDescription(const QString& firmware, const mcc::misc::TraitDescriptionList& traits);

                void startReadingParam(const QString& device, const QString& trait, int firstParamId, int paramCount);
                void stopReadingParam(const QString& device, const QString& trait, int firstParamId, int paramCount);
                void onProtocolForDevice(const QString& device, const QVector<mcc::misc::ProtocolId>& id);

            signals:
                void requestDevicesList();
                void requestDeviceActivate(const QString& device, bool isActive);
                void requestDeviceDescription(const QString& device);
                void requestFirmwareDescription(const QString& firmware);
                void requestCancelCmd(const QString& device, mcc::misc::CmdCollationId collationId);
                void requestDeviceUnregister(const QString& device);
                void requestProtocolForDevice(const QString& device);

                void requestDeviceFileUpload(const QString& service, const QString& device, const QString& filePath);
                void requestDeviceFileUploadCancel(const QString& service, const QString& device, const QString& filePath, const QString& reason);

                void subscribeTmParam(const mcc::misc::TmParam& tmParam);
                void sendCmdToExchange(const mcc::misc::Cmd& tmParam);

                void deviceAdded(FlyingDevice* model);
                void deviceRemoved(FlyingDevice* model);
                void deviceSignalBad(FlyingDevice* device);
                void deviceSignalGood(FlyingDevice* device);
                void deviceFirmwareLoaded(FlyingDevice* device);

                void deviceReadyForExchange(FlyingDevice* device);
                void deviceNotReadyForExchange(FlyingDevice* device);

                void selectionChanged(FlyingDevice* selectedDevice);

                void beginEditTemplate();
                void endEditTemplate(double delta, double height, double speed);
                void resetEditTemplate();
                void endEditRoute();

                void gotTmParamList(const QDateTime& time, const std::map<QString, mcc::misc::TmParam>& tmParamList);

                void cmdState(const QDateTime& time, mcc::misc::CmdCollationId collationId, mcc::misc::CmdState state, const QStringList& response, const QString& reason = QString());

                void groupAdded(DevicesGroup* group);
                void groupRemoved(DevicesGroup* group);

            private:
                void subscribeAllTm(FlyingDevice* device);

                virtual void timerEvent(QTimerEvent *) override;

                QMap<QString, FlyingDevice*> _devices;
                QVector<QPixmap>             _pixmaps;

                FlyingDevice*                _selectedDevice;

                QMap<mcc::misc::CmdCollationId, mcc::misc::Cmd> _sentCmds;

                typedef QVector<QString>              DevicesList;
                typedef QMap<QString, DevicesList>    DevicesInChannels;

                DevicesInChannels                     _devicesInChannels;
                DevicesList                           _devicesForExchange;

                bool _offlineMode;

                QMap<QString, ProtocolParamDescriptor> _protocolParams;

                std::vector<DevicesGroup*> _devicesGroups;
            };

        }
    }
}

