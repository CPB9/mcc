/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#pragma once

#include <memory>

#include <QSqlQuery>

#include "mcc/core/decode/Registry.h"
#include "mcc/core/decode/RegistryProvider.h"

namespace mcc
{
namespace decode
{

class Sqlite3RegistryProvider : public RegistryProvider
{
public:
    explicit Sqlite3RegistryProvider(std::string dbPath) : _dbPath(dbPath), _db(QSqlDatabase::addDatabase("QSQLITE")) {}
    std::unique_ptr<Registry> provide() override;
private:
    std::string _dbPath;
    QSqlDatabase _db;
    std::map<uint_fast64_t, std::shared_ptr<Namespace>> _namespaceById;
    std::map<uint_fast64_t, std::shared_ptr<Unit>> _unitById;
    std::map<uint_fast64_t, std::shared_ptr<Type>> _typeById;
    std::map<uint_fast64_t, std::shared_ptr<Component>> _componentById;

    QSqlQuery runQuery(const char *sql);
    QSqlQuery execQuery(QSqlQuery &query);
    
    std::shared_ptr<Type> ensureTypeLoaded(uint_fast64_t typeId);
    std::shared_ptr<Component> ensureComponentLoaded(uint_fast64_t componentId);
    std::shared_ptr<Component> loadComponent(uint_fast64_t componentId);
    Option<std::string> optionStringOf(QVariant variant);
};
}
}
