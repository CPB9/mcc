/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include <QSqlRecord>
#include <QSqlField>
#include <QSqlError>
#include <QVariant>
#include "bmcl/Result.h"
#include "mcc/messages/Device.h"
#include "mcc/messages/MessageSender.h"
#include "mcc/core/db/DbHandle.h"
#include "mcc/core/db/Sql.h"


#define get_or_exit_on_error(response, expression)                                                                     \
    ;                                                                                                                  \
    auto temp_##response = expression;                                                                                 \
    if (temp_##response.isErr())                                                                                       \
        return temp_##response.unwrapErr();                                                                            \
    auto response = temp_##response.unwrap();


namespace mcc {
namespace core {
namespace db {
namespace queries {

SqlResult findDevice(QSqlQuery* query, const std::string& device)
{
    query->prepare("select id from device where name = :name;");
    query->bindValue(":name", QString::fromStdString(device));
    auto r = execSelect(query);
    printIfErr(r);
    if (r.isSome())
        return r.take();

    while (query->next())
    {
        QVariant id = query->record().value(0);
        std::size_t i = id.toUInt();
        return id;
    }
    SqlError err;
    err.err = "no device found with name " + QString::fromStdString(device);
    err.query = query->lastQuery();
    err.binds = QString::fromStdString(device);
    return err;
}

bmcl::Option<SqlError> deleteTm(QSqlQuery* query, const QVariant& device_id)
{
    query->prepare("delete from mcc_tm where device_id = :device_id;");
    query->bindValue(":device_id", device_id);
    return execDelete(query);
}

bmcl::Option<SqlError> deleteCmd(QSqlQuery* query, const QVariant& device_id)
{
    query->prepare("delete from mcc_cmd where device_id = :device_id;");
    query->bindValue(":device_id", device_id);
    return execDelete(query);
}

bmcl::Option<SqlError> deleteCmdState(QSqlQuery* query, const QVariant& device_id)
{
    query->prepare("delete from mcc_cmd_state where device_id = :device_id;");
    query->bindValue(":device_id", device_id);
    return execDelete(query);
}

bmcl::Option<SqlError> deleteDeviceProtocol(QSqlQuery* query, const QVariant& device_id)
{
    query->prepare("delete from device_protocol where device_id = :device_id;");
    query->bindValue(":device_id", device_id);
    return execDelete(query);
}

bmcl::Option<SqlError> deleteDevice(QSqlQuery* query, const QVariant& device_id)
{
    query->prepare("delete from device where id = :device_id;");
    query->bindValue(":device_id", device_id);
    return execDelete(query);
}

void execute(DbHandle* db, const mcc::messages::MessageSender& sender, std::unique_ptr<mcc::messages::DeviceUnRegister_Request>&& request)
{
    QSqlQuery query = db->getQueryHandle();
    auto r = findDevice(&query, request->device());
    if (r.isErr())
    {
        sender->send<mcc::messages::DeviceUnRegistered>(request->device());
        sender->respond<mcc::messages::DeviceUnRegister_Response>(request.get(), r.takeErr().err.toStdString());
        return;
    }

    db->transaction();
    QVariant deviceId = r.take();
    printIfErr(deleteTm(&query, deviceId));
    printIfErr(deleteCmd(&query, deviceId));
    printIfErr(deleteCmdState(&query, deviceId));
    printIfErr(deleteDeviceProtocol(&query, deviceId));
    printIfErr(deleteDevice(&query, deviceId));
    db->commit();
    sender->send<mcc::messages::DeviceActionLog>("device", request->device(), "unregistered");
    sender->send<mcc::messages::DeviceUnRegistered>(request->device());
    sender->respond<mcc::messages::DeviceUnRegister_Response>(request.get());
}
}
}
}
}
