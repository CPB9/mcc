/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include <QSqlRecord>
#include <QVariant>

#include "mcc/messages/Firmware.h"
#include "mcc/messages/MessageSender.h"
#include "mcc/core/db/DbHandle.h"
#include "mcc/core/db/Sql.h"

namespace mcc {
namespace core {
namespace db {
namespace queries
{

class FirmwareList_Request
{
public:
    FirmwareList_Request(DbHandle* db, const mcc::messages::MessageSender& sender) : _sender(sender)
    {
        _query = db->getQueryHandle();
        bool r = _query.prepare("select name from firmware;");
        assert(r);
    }
    void execute(std::unique_ptr<mcc::messages::FirmwareList_Request>&& request)
    {
        printIfErr(execSelect(_query));
        std::vector<std::string> firmwares;
        while (_query.next())
        {
            firmwares.emplace_back(_query.record().value("name").toString().toStdString());
        }
        _sender->respond<mcc::messages::FirmwareList_Response>(request.get(), std::move(firmwares));
    }

private:
    QSqlQuery _query;
    mcc::messages::MessageSender _sender;
};


}
}
}
}