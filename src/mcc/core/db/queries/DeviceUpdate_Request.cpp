/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include <QSqlRecord>
#include <QSqlField>
#include <QSqlError>
#include <QSqlQuery>
#include <QVariant>

#include "mcc/messages/Device.h"
#include "mcc/messages/MessageSender.h"
#include "mcc/core/db/DbHandle.h"
#include "mcc/core/db/Sql.h"


namespace mcc {
namespace core {
namespace db {
namespace queries {

bool checkDiff(DbHandle* db, const std::string& device, const std::string& firmware, const std::string& kind)
{
    QSqlQuery query2 = db->getQueryHandle();
    query2.prepare("select device.name as device_name, firmware.name as firmware_name, device_kind.name as kind_name    \
                      from device   \
                           LEFT OUTER JOIN firmware    on device.firmware_id = firmware.id     \
                           LEFT OUTER JOIN device_kind on device.kind_id = device_kind.id  \
                     where device_name=:device;");

    query2.bindValue(":device", QString::fromStdString(device));
    printIfErr(execSelect(query2));
    query2.next();
    QSqlRecord r = query2.record();
    std::string d = r.value("device_name").toString().toStdString();
    std::string f = r.value("firmware_name").toString().toStdString();
    std::string k = r.value("kind_name").toString().toStdString();

    return (device != d || firmware != f || kind != k);
}

void execute(DbHandle* db, const mcc::messages::MessageSender& sender, std::unique_ptr<mcc::messages::DeviceUpdate_Request>&& request)
{
    if (!checkDiff(db, request->device(), request->firmware(), request->kind()))
    {
        sender->respond<mcc::messages::DeviceUpdate_Response>(request.get());
        return;
    }

    db->transaction();

    QSqlQuery query = db->getQueryHandle();
    query.prepare("update device set            \
                            updated=:updated,   \
                            firmware_id = (select id from firmware where name = :firmware), \
                            kind_id = (select id from device_kind where name = :kind)       \
                    where name=:device  \
                    ;");
    query.bindValue(":device", QString::fromStdString(request->device()));
    query.bindValue(":firmware", QString::fromStdString(request->firmware()));
    query.bindValue(":kind", QString::fromStdString(request->kind()));
    query.bindValue(":updated", QString::fromStdString(mcc::misc::currentDateTime()));
    printIfErr(execUpdate(query));

    if (checkDiff(db, request->device(), request->firmware(), request->kind()))
    {
        db->rollback();
        sender->respond<mcc::messages::DeviceUpdate_Response>(request.get(), "failed");
        return;
    }

    db->commit();
    sender->respond<mcc::messages::DeviceUpdate_Response>(request.get());
    sender->send<mcc::messages::DeviceUpdated>(request->device());
}

}
}
}
}