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
namespace queries
{

class DeviceList_Request
{
public:
    DeviceList_Request(DbHandle* db, const mcc::messages::MessageSender& sender) : _sender(sender)
    {
        _query = db->getQueryHandle();
        bool r = _query.prepare("select name from device;");
        assert(r);
    }
    void execute(std::unique_ptr<mcc::messages::DeviceList_Request>&& request)
    {
        printIfErr(execSelect(_query));
        std::vector<std::string> devices;
        while (_query.next())
        {
            devices.emplace_back(_query.record().value("name").toString().toStdString());
        }
        _sender->respond<mcc::messages::DeviceList_Response>(request.get(), std::move(devices));
    }

private:
    QSqlQuery _query;
    mcc::messages::MessageSender _sender;
};


}
}
}
}