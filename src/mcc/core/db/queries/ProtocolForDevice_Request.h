/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include <QSqlRecord>
#include <QSqlQuery>

#include "mcc/messages/Protocol.h"
#include "mcc/misc/Protocol.h"
#include "mcc/messages/MessageSender.h"
#include "mcc/core/db/DbHandle.h"
#include "mcc/core/db/Sql.h"



namespace mcc {
namespace core {
namespace db {
namespace queries {

class ProtocolForDevice_Request
{
public:
    ProtocolForDevice_Request(DbHandle* db, const mcc::messages::MessageSender& sender) : _sender(sender)
    {
        _query = db->getQueryHandle();
        _query.prepare("select protocol.name                  as protocol       \
                             , device_protocol.protocol_value as param_value    \
                          from device_protocol                                  \
                             , protocol                                         \
                         where device_protocol.protocol_id = protocol.id        \
                           and device_protocol.device_id = (select id from device where name=:device)   \
                             ;"); 
    }
    void execute(std::unique_ptr<mcc::messages::ProtocolForDevice_Request>&& request)
    {
        _query.bindValue(":device", QString::fromStdString(request->device()));
        printIfErr(execSelect(_query));

        std::vector<mcc::misc::ProtocolId> ps;
        while (_query.next())
        {
            QSqlRecord r = _query.record();
            ps.emplace_back(r.value("protocol").toString(), r.value("param_value").toUInt());
        }

        _sender->respond<mcc::messages::ProtocolForDevice_Response>(request.get(), std::move(ps));
    }

private:
    QSqlQuery _query;
    mcc::messages::MessageSender _sender;
};



}
}
}
}