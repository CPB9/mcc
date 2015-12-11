/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include <QSqlRecord>
#include <QSqlField>
#include <QSqlError>
#include <QSqlQuery>
#include <QVariant>
#include <QUuid>
#include "bmcl/Result.h"

#include "mcc/messages/Device.h"
#include "mcc/messages/Firmware.h"
#include "mcc/messages/MessageSender.h"
#include "mcc/core/db/DbHandle.h"
#include "mcc/core/db/Sql.h"


#define get_or_exit_on_error(response, expression)                                                                     \
    ;                                                                                                                  \
    auto temp_##response = expression;                                                                                 \
    if (temp_##response.isErr())                                                                                       \
        return temp_##response.unwrapErr();                                                                            \
    auto response = temp_##response.unwrap();

#define exit_on_error(expression)                                                                                      \
    ;                                                                                                                  \
    auto temp_##response = expression;                                                                                 \
    if (temp_##response.isSome())                                                                                      \
        return temp_##response.take();                                                                                 \


namespace mcc {
namespace core {
namespace db {
namespace queries {

SqlResult insertDevice(QSqlQuery& query, const QString& name, const QString& info)
{
    QString time = QString::fromStdString(mcc::misc::currentDateTime());
    query.prepare("insert into device (name, info, registered, updated) values (:name, :info, :registered, :updated)");
    query.bindValue(":name", name);
    query.bindValue(":info", info);
    query.bindValue(":registered", time);
    query.bindValue(":updated", time);
    return execInsert(query);
}

void execute(DbHandle* db, const mcc::messages::MessageSender& sender, std::unique_ptr<mcc::messages::DeviceRegister_Request>&& request)
{
    QSqlQuery query = db->getQueryHandle();
    QString name = QUuid::createUuid().toString();
    auto r = insertDevice(query, name, QString::fromStdString(request->info()));
    if (r.isErr())
    {
        sender->respond<mcc::messages::DeviceRegister_Response>(request.get(), "Unable to register device");
        return;
    }

    std::size_t id = r.take().toULongLong();
    sender->send<mcc::messages::DeviceActionLog>("device", name.toStdString(), "registered");
    sender->send<mcc::messages::DeviceRegistered>(id, name.toStdString(), request->info());
    sender->respond<mcc::messages::DeviceRegister_Response>(request.get(), id, name.toStdString());
}


}
}
}
}
