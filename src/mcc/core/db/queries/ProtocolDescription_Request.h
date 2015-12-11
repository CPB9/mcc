/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include <QSqlRecord>
#include <QSqlField>
#include <QSqlError>
#include <QVariant>

#include "mcc/messages/Protocol.h"
#include "mcc/messages/MessageSender.h"
#include "mcc/core/db/DbHandle.h"
#include "mcc/core/db/Sql.h"


namespace mcc {
namespace core {
namespace db {
namespace queries {

class ProtocolDescription_Request
{
public:
    ProtocolDescription_Request(DbHandle* db, const mcc::messages::MessageSender& sender) : _sender(sender)
    {
        _query = db->getQueryHandle();
        bool r = _query.prepare("select protocol.id                         \
                                        , protocol.name                     \
                                        , protocol.info                     \
                                        , protocol.param_info               \
                                        , trait.name        as trait_name   \
                                        , service.name      as service_name \
                                        from protocol                       \
                                        , trait                             \
                                        , service                           \
                                    where protocol.name = :name             \
                                        and trait.id   = protocol.trait_id    \
                                        and service.id = protocol.service_id  \
                                        ;");
        assert(r);
    }
    void execute(std::unique_ptr<mcc::messages::ProtocolDescription_Request>&& request)
    {
        _query.bindValue(":name", QString::fromStdString(request->protocol()));
        auto p = execSelect(_query);
        if (p.isSome())
        {
            auto err = p.take();
            _sender->respond<mcc::messages::ProtocolDescription_Response>(request.get(), err.err.toStdString());
            return ;
        }

        _query.next();
        QSqlRecord r = _query.record();
        mcc::messages::ProtocolDescription d(r.value("id").toUInt()
            , request->protocol()
            , r.value("info").toString().toStdString()
            , r.value("param_info").toString().toStdString()
            , r.value("trait_name").toString().toStdString()
            , r.value("service_name").toString().toStdString()
            );

        _sender->respond<mcc::messages::ProtocolDescription_Response>(request.get(), d);
    }

private:
    QSqlQuery _query;
    mcc::messages::MessageSender _sender;
};




}
}
}
}