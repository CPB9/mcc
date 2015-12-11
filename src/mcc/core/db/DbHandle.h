/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#pragma once
#include <assert.h>
#include <QSqlDatabase>
#include <QFile>
#include <QDebug>
#include <QSqlQuery>
#include <QSqlError>
#include <QSqlRecord>
#include <QSqlField>



namespace mcc {
namespace core {
namespace db {

class DbHandle
{
public:
    QString version() const
    {
        if (!_db.isOpen())
            return QString();
        QSqlQuery query(_db);
        query.exec("select sqlite_version();");
        query.next();
        return query.record().field(0).value().toString();
    }

    void speedupSqlite()
    {
        QSqlQuery query(_db);
        query.exec("PRAGMA synchronous = OFF");
        query.exec("PRAGMA page_size = 4096");
        query.exec("PRAGMA cache_size = 16384");
        query.exec("PRAGMA temp_store = MEMORY");
        query.exec("PRAGMA journal_mode = MEMORY");
    }

    bool create(const QString& path, const QString& schema_path)
    {
        if (QFile::exists(path))
        {
            qDebug() << "Database already exists: " << path << ", removed" << endl;
            QFile::remove(path);
            //return true;
        }

        QFile schema(schema_path);
        if (!schema.open(QIODevice::ReadOnly | QIODevice::Text))
        {
            qDebug() << "Unable to load database schema: " << schema.fileName() << " because of error " << schema.errorString() << endl;
            return false;
        }

        QSqlDatabase db;
        if (!createDbHandle(path, db))
            return false;

        if (!db.open())
        {
            qDebug() << "Unable to create database (open): " << path << endl;
            schema.close();
            return false;
        }

        db.transaction();
        QSqlQuery query(db);
        if (!executeQueriesFromFile(&schema, &query))
        {
            db.rollback();
            qDebug() << "Unable to create database schema: " << schema.fileName() << " because of error " << db.lastError() << endl;
            return false;
        }
        db.commit();

        schema.close();
        query.clear();

        _db = db;
        return true;
    }

    bool start(const QString& path)
    {
        if (!QFile::exists(path))
            return false;

        if (!createDbHandle(path, _db))
            return false;
        _db.open();

        return true;
    }

    void transaction()
    {
        _db.commit();
        _db.transaction();
    }

    void commit()
    {
        _db.commit();
    }

    void rollback()
    {
        _db.rollback();
    }

    QSqlQuery getQueryHandle()
    {
        return QSqlQuery(_db);
    }

    void stop()
    {
        _path.clear();
        _db.close();
    }

    virtual ~DbHandle()
    {
        stop();
    }

private:
    static bool createDbHandle(const QString& path, QSqlDatabase& db)
    {
        db = QSqlDatabase::addDatabase("QSQLITE", "mcc");

        db.setHostName(QString());
        db.setUserName(QString());
        db.setPassword(QString());
        db.setDatabaseName(path);

        if (!db.isValid())
        {
            qDebug() << "Unable to create database handle: " << path << endl;
            return false;
        }

        return true;
    }

    static bool executeQueriesFromFile(QFile* file, QSqlQuery* query)
    {
        while (!file->atEnd())
        {
            QString cleanedLine;
            QString line;

            while (true)
            {
                cleanedLine = file->readLine().trimmed();
                // remove comments at end of line
                QStringList strings = cleanedLine.split("--");
                cleanedLine = strings.at(0);

                // remove lines with only comment, and DROP lines
                if (!cleanedLine.startsWith("--")
                    && !cleanedLine.startsWith("DROP")
                    && !cleanedLine.isEmpty())
                {
                    line += cleanedLine;
                }

                if (cleanedLine.endsWith(";") || cleanedLine.startsWith("COMMIT") || file->atEnd())
                {
                    break;
                }
            }

            if (line.isEmpty())
            {
                continue;
            }

            if (!query->exec(line))
            {
                query->clear();
                assert(false);
                return false;
            }

            query->clear();
            //qDebug() << QSqlDatabase::drivers();
            //qDebug() << query->lastError();
            //qDebug() << "test executed query:" << query->executedQuery();
            //qDebug() << "test last query:" << query->lastQuery();
        }

        return true;
    }

private:
    QString      _path;
    QSqlDatabase _db;
};
}
}
}
