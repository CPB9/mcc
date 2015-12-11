/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#pragma once
#include <QSqlQuery>
#include <QDateTime>

#include "mcc/messages/Cmd.h"
#include "mcc/core/db/DbHandle.h"
#include "mcc/core/db/Sql.h"


namespace mcc {
namespace core {
namespace db {
namespace queries
{


class Cmd
{
public:
    Cmd(DbHandle* db)
    {
        _query = db->getQueryHandle();
        _query.prepare("insert into mcc_cmd(device_id   \
                                            , time      \
                                            , trait     \
                                            , name      \
                                            , params    \
                                            , collation_id \
                                            )           \
                                    values  ( (select id from device where name = :device)  \
                                            , :time     \
                                            , :trait    \
                                            , :name     \
                                            , :params   \
                                            , :collation_id \
                                            );          \
                        ");
    }
    void execute(std::unique_ptr<mcc::messages::Cmd>&& cmd)
    {
        _query.bindValue(":device", QString::fromStdString(cmd->device()));
        _query.bindValue(":trait", QString::fromStdString(cmd->trait()));
        _query.bindValue(":name", QString::fromStdString(cmd->command()));
        _query.bindValue(":params", QString::fromStdString(cmd->paramsAsString(",")));
        _query.bindValue(":time", QString::fromStdString(cmd->time()));
        _query.bindValue(":collation_id", cmd->cmdId());
        auto id = execInsert(_query);
    }

private:
    QSqlQuery _query;
};

}
}
}
}