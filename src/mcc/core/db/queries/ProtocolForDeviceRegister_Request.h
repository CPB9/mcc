/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

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

class ProtocolForDeviceRegister_Request
{
public:
    ProtocolForDeviceRegister_Request(DbHandle* db, const mcc::messages::MessageSender& sender) : _sender(sender)
    {
        _query = db->getQueryHandle();
        bool r = _query.prepare("insert into device_protocol  \
                                        (                       \
                                            protocol_value,     \
                                            protocol_id,        \
                                            device_id           \
                                        )                       \
                                        values                  \
                                        (                       \
                                            :protocol_value,    \
                                            (select id from protocol where name = :protocol_name),  \
                                            (select id from device where name = :device)            \
                                        )   \
                            ;");
        assert(r);
    }
    void execute(std::unique_ptr<mcc::messages::ProtocolForDeviceRegister_Request>&& request)
    {
        _query.bindValue(":device", QString::fromStdString(request->device()));
        _query.bindValue(":protocol_name", request->protocol().protocol());
        _query.bindValue(":protocol_value", (qulonglong)request->protocol().id());
        auto r = execInsert(_query);
        if (r.isErr())
        {
            auto err = r.unwrapErr();
            _sender->respond<mcc::messages::ProtocolForDeviceRegister_Response>(request.get(), "failed to register: id already owned");
            return;
        }
        _sender->respond<mcc::messages::ProtocolForDeviceRegister_Response>(request.get());
    }

private:
    QSqlQuery _query;
    mcc::messages::MessageSender _sender;
};


}
}
}
}