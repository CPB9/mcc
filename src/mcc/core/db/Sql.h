/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#pragma once
#include <QSqlError>
#include <QSqlQuery>
#include <QDebug>

#include "bmcl/Option.h"
#include "bmcl/Result.h"


namespace mcc {
namespace core {
namespace db {

struct SqlError
{
    SqlError() = default;
    SqlError(const std::string& error) :err(QString::fromStdString(error)){}
    QString query;
    QString binds;
    QString err;
};

typedef bmcl::Result<QVariant, SqlError> SqlResult;

inline void print(const SqlError& err)
{
    qDebug() << err.err;
    qDebug() << err.query;
    qDebug() << err.binds;
}

inline void printIfErr(const SqlResult& data)
{
    if (data.isOk())
        return;
    print(data.unwrapErr());
}

inline void printIfErr(const bmcl::Option<SqlError>& data)
{
    if (data.isNone())
        return;
    print(data.unwrap());
}

inline bmcl::Option<SqlError> checkForError(const QSqlQuery* query, bool check = true)
{
    QSqlError err = query->lastError();
    if (err.type() == QSqlError::NoError)
        return bmcl::None;

    SqlError r;
    if (!check)
        return r;
    r.err   = err.text();
    r.query = query->lastQuery();

    QMapIterator<QString, QVariant> i(query->boundValues());
    while (i.hasNext())
    {
        i.next();
        r.binds = QString("%1 : %2").arg(i.key()).arg(i.value().toString());
    }
    //print(r);
    return r;
}

inline bmcl::Option<SqlError> exec(QSqlQuery* query, bool check = true)
{
    if (query->exec())
        return bmcl::None;

    return checkForError(query, check).take();
}

inline SqlResult execInsert(QSqlQuery* query, bool check = true)
{
    auto r = exec(query, check);
    if (r.isSome())
        return r.take();
    return query->lastInsertId();
}

inline bmcl::Option<SqlError> execSelect(QSqlQuery* query, bool check = true)
{
    return exec(query, check);
}

inline bmcl::Option<SqlError> execDelete(QSqlQuery* query, bool check = true)
{
    return exec(query, check);
}

inline bmcl::Option<SqlError> execUpdate(QSqlQuery* query, bool check = true)
{
    return exec(query, check);
}

inline SqlResult execInsert(QSqlQuery& query, bool check = true)
{
    return execInsert(&query, check);
}

inline bmcl::Option<SqlError> execSelect(QSqlQuery& query, bool check = true)
{
    return execSelect(&query, check);
}

inline bmcl::Option<SqlError> execDelete(QSqlQuery& query, bool check = true)
{
    return execDelete(&query, check);
}

inline bmcl::Option<SqlError> execUpdate(QSqlQuery& query, bool check = true)
{
    return execUpdate(&query, check);
}

}
}
}