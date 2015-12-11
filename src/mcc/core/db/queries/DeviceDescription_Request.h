/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#pragma once
#include <QSqlRecord>
#include <QSqlField>

#include "mcc/messages/Device.h"
#include "mcc/messages/MessageSender.h"
#include "mcc/core/db/DbHandle.h"
#include "mcc/core/db/Sql.h"


namespace mcc {
namespace core {
namespace db {
namespace queries {

class DeviceDescription_Request
{
public:
    DeviceDescription_Request(DbHandle* db, const mcc::messages::MessageSender& sender)
        : _sender(sender)
    {
        _query = db->getQueryHandle();
        bool r = _query.prepare("select device.id              as device_id         \
                                      , device.info          as device_info         \
                                      , device.registered    as device_registered   \
                                      , device.updated       as device_updated      \
                                      , device_kind.id       as kind_id             \
                                      , device_kind.name     as kind_name           \
                                      , device_kind.info     as kind_info           \
                                      , firmware.id          as firmware_id         \
                                      , firmware.name        as firmware_name       \
                                      , firmware.info        as firmware_info       \
                                      , firmware.source      as firmware_source     \
                                      , firmware.registered  as firmware_registered \
                                    from device                                                              \
                                         LEFT OUTER JOIN firmware    on device.firmware_id = firmware.id     \
                                         LEFT OUTER JOIN device_kind on device.kind_id     = device_kind.id  \
                                    where device.name = :device_name;");
        if (!r)
        {
            printIfErr(checkForError(&_query));
            assert(r);
        }
    }
    void execute(std::unique_ptr<mcc::messages::DeviceDescription_Request>&& request)
    {
        _query.bindValue(":device_name", QString::fromStdString(request->device()));
        printIfErr(execSelect(_query));

        if (!_query.next())
        {
            _sender->respond<mcc::messages::DeviceDescription_Response>(request.get(), "device not found");
            return;
        }

        QSqlRecord r = _query.record();
        std::size_t device_id = r.value("device_id").toUInt();
        std::string device_info = r.value("device_info").toString().toStdString();
        std::string device_registered = r.value("device_registered").toString().toStdString();
        std::string device_updated = r.value("device_updated").toString().toStdString();

        std::size_t kind_id   = r.value("kind_id").toString().toUInt();
        std::string kind_name = r.value("kind_name").toString().toStdString();
        std::string kind_info = r.value("kind_info").toString().toStdString();

        std::size_t firmware_id = r.value("firmware_id").toUInt();
        std::string firmware_name = r.value("firmware_name").toString().toStdString();
        std::string firmware_info = r.value("firmware_info").toString().toStdString();
        std::string firmware_source = r.value("firmware_source").toString().toStdString();
        std::string firmware_registered = r.value("firmware_registered").toString().toStdString();
        _sender->respond<mcc::messages::DeviceDescription_Response>(request.get(), device_id, device_info, kind_id, kind_name, kind_info, firmware_id, firmware_name, firmware_info);
    }

private:
    QSqlQuery _query;
    mcc::messages::MessageSender _sender;
};


}
}
}
}