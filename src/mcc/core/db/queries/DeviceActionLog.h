/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#pragma once
#include "mcc/messages/Device.h"
#include "mcc/core/db/DbHandle.h"
#include "mcc/core/db/Sql.h"


namespace mcc {
namespace core {
namespace db {
namespace queries
{

class DeviceActionLog
{
public:
    DeviceActionLog(DbHandle* db)
    {
        _query = db->getQueryHandle();
        _query.prepare("insert into mcc_action( kind    \
                                            , time      \
                                            , name      \
                                            , action    \
                                            , details   \
                                            )           \
                                    values  ( :kind     \
                                            , :time     \
                                            , :name     \
                                            , :action   \
                                            , :details  \
                                            );          \
                        ");
        }
    void execute(std::unique_ptr<mcc::messages::DeviceActionLog>&& log)
    {
        _query.bindValue(":time", QString::fromStdString(log->time()));
        _query.bindValue(":kind", QString::fromStdString(log->kind()));
        _query.bindValue(":name", QString::fromStdString(log->name()));
        _query.bindValue(":action", QString::fromStdString(log->action()));
        _query.bindValue(":details", QString::fromStdString(log->details()));
        auto id = execInsert(_query);
    }

private:
    QSqlQuery _query;
};

}
}
}
}