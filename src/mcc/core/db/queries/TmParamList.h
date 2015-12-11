/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#pragma once
#include <QSqlQuery>

#include "mcc/messages/Tm.h"
#include "mcc/core/db/DbHandle.h"
#include "mcc/core/db/Sql.h"


namespace mcc {
namespace core {
namespace db {
namespace queries {

class TmParamList
{
public:
    TmParamList(DbHandle* db)
    {
        _query = db->getQueryHandle();
        bool r = _query.prepare("insert into mcc_tm ( time       \
                                            , value     \
                                            , device_id \
                                            , param_id  \
                                            )           \
                                    values  ( :time     \
                                            , :value    \
                                            , (select id from device where name = :device)  \
                                            ,   (select trait_field.id                      \
                                                    from trait                              \
                                                        , trait_field                       \
                                                    where trait.firmware_id = (select firmware_id from device where name = :device) \
                                                    and trait.unique_name    =:trait        \
                                                    and trait_field.trait_id =trait.id      \
                                                    and trait_field.name     =:name         \
                                                )       \
                                            );          \
                        ");
        assert(r);
    //     query.prepare("insert into mcc_tm(device_id \
    //                                     , time      \
    //                                     , trait     \
    //                                     , name      \
    //                                     , value     \
    //                                     )           \
    //                             values  ( (select id from device where name = :device)  \
    //                                     , :time     \
    //                                     , :trait    \
    //                                     , :name     \
    //                                     , :value    \
    //                                     );          \
    //                     ");
        }
    void execute(std::unique_ptr<mcc::messages::TmParamList>&& list)
    {

        for (const auto& p : list->params())
        {
            _query.bindValue(":device", QString::fromStdString(list->device()));
            _query.bindValue(":trait", QString::fromStdString(p.trait()));
            _query.bindValue(":name", QString::fromStdString(p.status()));
            _query.bindValue(":value", p.value().toQVariant());
            _query.bindValue(":time", QString::fromStdString(list->time()));
            auto id = execInsert(_query, false);
            //if (id.isOk())
            //    qDebug() << id.take();
            //printIfErr(execInsert(query));
        }
    }
private:
    QSqlQuery _query;
};


}
}
}
}