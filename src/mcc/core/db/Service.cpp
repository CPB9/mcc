/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include <chrono>
#include <QCoreApplication>
#include <QResource>
#include <QDateTime>

#include "mcc/core/db/Service.h"
#include "mcc/core/db/FirmwareLoader.h"
#include "mcc/Names.h"

#include "mcc/core/db/queries/Cmd.h"
#include "mcc/core/db/queries/CmdState.h"
#include "mcc/core/db/queries/TmParamList.h"
#include "mcc/core/db/queries/DeviceActionLog.h"
#include "mcc/core/db/queries/DeviceDescription_Request.h"
#include "mcc/core/db/queries/DeviceList_Request.h"
#include "mcc/core/db/queries/FirmwareList_Request.h"
#include "mcc/core/db/queries/ProtocolForDevice_Request.h"
#include "mcc/core/db/queries/ProtocolForDeviceRegister_Request.h"
#include "mcc/core/db/queries/ProtocolList_Request.h"
#include "mcc/core/db/queries/ProtocolDescription_Request.h"


namespace mcc {
namespace core {
namespace db {

namespace queries
{
    extern void execute(DbHandle* db, const mcc::messages::MessageSender& sender, std::unique_ptr<mcc::messages::FirmwareDescription_Request>&&);
    extern void execute(DbHandle* db, const mcc::messages::MessageSender& sender, std::unique_ptr<mcc::messages::FirmwareRegister_Request>&&);
    extern void execute(DbHandle* db, const mcc::messages::MessageSender& sender, std::unique_ptr<mcc::messages::DeviceUpdate_Request>&&);
    extern void execute(DbHandle* db, const mcc::messages::MessageSender& sender, std::unique_ptr<mcc::messages::DeviceRegister_Request>&&);
    extern void execute(DbHandle* db, const mcc::messages::MessageSender& sender, std::unique_ptr<mcc::messages::DeviceUnRegister_Request>&&);
    extern void execute(DbHandle* db, const mcc::messages::MessageSender& sender, std::unique_ptr<mcc::messages::ProtocolForDeviceRegister_Request>&&);
}

Service::Service(const mcc::messages::LocalRouterPtr& router) : mcc::messages::ServiceAbstract(mcc::Names::coreDb(), router)
{
}

Service::~Service()
{
    finish(true);
}

bool Service::pre()
{
    QString mccDbPath = QCoreApplication::applicationDirPath() + "/mcc.sqlite";

    if (!QFile::exists(mccDbPath))
    {
        if (!_db.create(mccDbPath, QString::fromStdString(_dbSchema)))
        {
            assert(false);
            return false;
        }

        _db.transaction();
        if (!FirmwareLoader::load(QString::fromStdString(_dirTraits), _db.getQueryHandle()))
        {
            _db.rollback();
            assert(false);
            _db.stop();
            return false;
        }
        _db.commit();
    }
    else
    {
        _db.start(mccDbPath);
    }
    qDebug() << "sqlite version: "<< _db.version();
    _db.speedupSqlite();

    _cmd = misc::makeUnique<queries::Cmd>(&_db);
    _cmdstate = misc::makeUnique<queries::CmdState>(&_db);
    _tmparamlist = misc::makeUnique<queries::TmParamList>(&_db);
    _actions = misc::makeUnique<queries::DeviceActionLog>(&_db);
    _devicelist = misc::makeUnique<queries::DeviceList_Request>(&_db, _out);
    _devicedescription = misc::makeUnique<queries::DeviceDescription_Request>(&_db, _out);
    _protocolfordevice = misc::makeUnique<queries::ProtocolForDevice_Request>(&_db, _out);
    _protocolfordeviceregister = misc::makeUnique<queries::ProtocolForDeviceRegister_Request>(&_db, _out);
    _protocollist = misc::makeUnique<queries::ProtocolList_Request>(&_db, _out);
    _protocoldescription = misc::makeUnique<queries::ProtocolDescription_Request>(&_db, _out);
    _firmwarelist = misc::makeUnique<queries::FirmwareList_Request>(&_db, _out);
    return ServiceAbstract::pre();
}

void Service::post()
{
    _db.commit();
    _db.stop();
    return ServiceAbstract::post();
}

void Service::tick()
{
    auto m = _in->tryRecvFor(std::chrono::milliseconds(100));
    if (m.isOk())
        chooseProcessor(m.take());

    static std::chrono::steady_clock::time_point start = std::chrono::steady_clock::now();
    if (std::chrono::duration_cast<std::chrono::seconds>(std::chrono::steady_clock::now() - start).count() >= 10)
    {
        _db.transaction();
        start = std::chrono::steady_clock::now();
    }
}

void Service::process(std::unique_ptr<mcc::messages::Cmd>&& message)
{
    _cmd->execute(std::move(message));
}

void Service::process(std::unique_ptr<mcc::messages::CmdState>&& message)
{
    _cmdstate->execute(std::move(message));
}

void Service::process(std::unique_ptr<mcc::messages::DeviceDescription_Request>&& request)
{
    _devicedescription->execute(std::move(request));
}

void Service::process(std::unique_ptr<mcc::messages::DeviceUpdate_Request>&& request)
{
    queries::execute(&_db, _out, std::move(request));
}

void Service::process(std::unique_ptr<mcc::messages::TmParamList>&& message)
{
    _tmparamlist->execute(std::move(message));
}

void Service::process(std::unique_ptr<mcc::messages::DeviceUnRegister_Request>&& request)
{
    queries::execute(&_db, _out, std::move(request));
}

void Service::process(std::unique_ptr<mcc::messages::DeviceActionLog>&& message)
{
    _actions->execute(std::move(message));
}

void Service::process(std::unique_ptr<mcc::messages::FirmwareList_Request>&& message)
{
    _firmwarelist->execute(std::move(message));
}

void Service::process(std::unique_ptr<mcc::messages::FirmwareDescription_Request>&& request)
{
    queries::execute(&_db, _out, std::move(request));
}

void Service::process(std::unique_ptr<mcc::messages::FirmwareRegister_Request>&& request)
{
    queries::execute(&_db, _out, std::move(request));
}

void Service::process(std::unique_ptr<mcc::messages::ProtocolList_Request>&& message)
{
    _protocollist->execute(std::move(message));
}

void Service::process(std::unique_ptr<mcc::messages::ProtocolDescription_Request>&& message)
{
    _protocoldescription->execute(std::move(message));
}

void Service::process(std::unique_ptr<mcc::messages::ProtocolForDevice_Request>&& message)
{
    _protocolfordevice->execute(std::move(message));
}

void Service::process(std::unique_ptr<mcc::messages::ProtocolForDeviceRegister_Request>&& message)
{
    _protocolfordeviceregister->execute(std::move(message));
}

void Service::process(std::unique_ptr<mcc::messages::DeviceRegister_Request>&& request)
{
    queries::execute(&_db, _out, std::move(request));
}

void Service::process(std::unique_ptr<mcc::messages::DeviceList_Request>&& message)
{
    _devicelist->execute(std::move(message));
}

}
}
}
