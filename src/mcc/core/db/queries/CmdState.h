/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#pragma once
#include <QSqlQuery>

#include "mcc/messages/Cmd.h"
#include "mcc/core/db/DbHandle.h"
#include "mcc/core/db/Sql.h"


namespace mcc {
namespace core {
namespace db {
namespace queries {

class CmdState
{
public:
    CmdState(DbHandle* db)
    {
        _query = db->getQueryHandle();
        _query.prepare("insert into mcc_cmd_state(device_id, collation_id, time, state, reason)           \
                                                      values  ( (select id from device where name = :device), :collation_id, :time, :state, :reason);");
    }
    void execute(std::unique_ptr<mcc::messages::CmdState>&& state)
    {
        _query.bindValue(":device", QString::fromStdString(state->device()));
        _query.bindValue(":collation_id", state->cmdId());
        _query.bindValue(":state", state->state());
        _query.bindValue(":time", QString::fromStdString(state->time()));
        _query.bindValue(":reason", QString::fromStdString(state->reason()));
        auto id = execInsert(_query);
    }

private:
    QSqlQuery _query;
};

}
}
}
}