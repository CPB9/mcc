/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include <QSqlDatabase>
#include <QSqlError>
#include <QVariant>
#include <QCoreApplication>
#include <QDir>
#include <QDebug>

#include "bmcl/Assert.h"
#include "bmcl/Panic.h"

#include "mcc/misc/cast_utils.h"

#include "mcc/core/decode/Sqlite3RegistryProvider.h"

namespace mcc
{
namespace decode
{

std::unique_ptr<Registry> Sqlite3RegistryProvider::provide()
{
    _db.setDatabaseName(QString::fromStdString(_dbPath));
    if (!_db.isValid() || !_db.open())
    {
        bmcl::panic(QString("%1:%2 %3 (dbPath=%4)").arg(__FILE__).arg(__LINE__)
            .arg(_db.lastError().text()).arg(QString::fromStdString(_dbPath)).toStdString().c_str());
    }

    QSqlQuery namespacesQuery(runQuery("SELECT id, name FROM namespace"));
    while (namespacesQuery.next())
    {
        _namespaceById.emplace(namespacesQuery.value("id").toULongLong(),
                               std::make_shared<Namespace>(namespacesQuery.value("name").toString().toStdString(),
                                                           std::weak_ptr<Namespace>()));
    }

    QSqlQuery subNamespacesQuery(runQuery("SELECT namespace_id, sub_namespace_id FROM sub_namespace"));
    while (subNamespacesQuery.next())
    {
        std::shared_ptr<Namespace> & parentNamespace = _namespaceById.at(subNamespacesQuery.value("namespace_id").toULongLong()),
                & childNamespace = _namespaceById.at(subNamespacesQuery.value("sub_namespace_id").toULongLong());
        childNamespace->setParentNamespace(parentNamespace);
        parentNamespace->subNamespaces().emplace_back(childNamespace);
    }

    QSqlQuery unitsQuery(runQuery("SELECT id, namespace_id, name, display, info FROM unit"));
    while (unitsQuery.next())
    {
        uint_fast64_t namespaceId(unitsQuery.value("id").toULongLong());
        std::shared_ptr<Namespace> & _namespace = _namespaceById.at(namespaceId);
        std::shared_ptr<Unit> unit = std::make_shared<Unit>(unitsQuery.value("namespace_id").toString().toStdString(), _namespace,
                            unitsQuery.value("name").toString().toStdString(), optionStringOf(unitsQuery.value("display")));
        _namespace->units().emplace_back(unit);
        _unitById.emplace(unitsQuery.value("id").toULongLong(), unit);
    }

    QSqlQuery typesQuery(runQuery("SELECT id FROM type"));
    while (typesQuery.next())
    {
        ensureTypeLoaded(typesQuery.value("id").toULongLong());
    }
    
    QSqlQuery componentsQuery(runQuery("SELECT id FROM component"));
    while (componentsQuery.next())
    {
        ensureComponentLoaded(componentsQuery.value("id").toULongLong());
    }
    
    for(auto& type : _typeById)
    {
        type.second->updateIndices();
    }
    
    for(auto& component : _componentById)
    {
        component.second->updateIndices();
    }
    
    std::vector<std::shared_ptr<Namespace>> rootNamespaces;
    for (auto& _namespace : _namespaceById)
    {
        _namespace.second->updateIndices();
        if (!_namespace.second->parentNamespace())
            rootNamespaces.emplace_back(_namespace.second);
    }

    return std::unique_ptr<Registry>(new Registry(std::move(rootNamespaces)));
}

std::shared_ptr<Component> Sqlite3RegistryProvider::ensureComponentLoaded(uint_fast64_t componentId)
{
    auto it(_componentById.find(componentId));
    if (it != _componentById.end())
        return it->second;
    return loadComponent(componentId);
}

std::shared_ptr<Type> Sqlite3RegistryProvider::ensureTypeLoaded(uint_fast64_t typeId)
{
    auto it(_typeById.find(typeId));
    if (it != _typeById.end())
        return it->second;
    QSqlQuery sql;
    sql.prepare("SELECT t.namespace_id AS namespace_id, t.name AS name, t.info AS info, p.kind AS kind, p.bit_length AS bit_length,"
    " a.base_type_id AS a_base_type_id, s.base_type_id AS s_base_type_id, e.base_type_id AS e_base_type_id,"
    " ar.base_type_id AS ar_base_type_id, ar.min_length AS min_length, ar.max_length AS max_length, str.id AS str_type_id"
    " FROM type AS t LEFT JOIN primitive_type AS p ON p.type_id = t.id"
    " LEFT JOIN alias_type AS a ON a.type_id = t.id LEFT JOIN sub_type AS s ON s.type_id = t.id LEFT JOIN enum_type AS e ON e.type_id = t.id"
    " LEFT JOIN array_type AS ar ON ar.type_id = t.id LEFT JOIN struct_type AS str ON str.type_id = t.id WHERE t.id = :type_id");
    sql.bindValue(":type_id", QVariant::fromValue(typeId));
    execQuery(sql);
    if (!sql.next())
    {
        bmcl::panic(QString("type not found in DB for id '%1'").arg(typeId).toStdString().c_str());
    }
    auto & _namespace = _namespaceById.at(sql.value("namespace_id").toULongLong());
    std::shared_ptr<Type> type;
    auto nameOptional(optionStringOf(sql.value("name")));
    auto infoOptional(optionStringOf(sql.value("info")));
    
    QVariant primitiveKind(sql.value("kind"));
    if (!primitiveKind.isNull())
    {
        const std::string &typeKindStr(primitiveKind.toString().toStdString());
        type = std::make_shared<PrimitiveType>(nameOptional, _namespace, infoOptional,
                                 primitiveTypeKindOf(typeKindStr),
                                 utils::checked_cast<uint_fast8_t>(sql.value("bit_length").toUInt()));
    }
    
    QVariant aliasBaseTypeIdVariant(sql.value("a_base_type_id"));
    if (!aliasBaseTypeIdVariant.isNull())
    {
        BMCL_ASSERT_MSG(!type, "invalid type");
        type = std::make_shared<AliasType>(nameOptional, _namespace, infoOptional, ensureTypeLoaded(aliasBaseTypeIdVariant.toULongLong()));
    }
    
    QVariant subTypeBaseTypeIdVariant(sql.value("s_base_type_id"));
    if (!subTypeBaseTypeIdVariant.isNull())
    {
        BMCL_ASSERT_MSG(!type, "invalid type");
        type = std::make_shared<SubType>(nameOptional, _namespace, infoOptional, ensureTypeLoaded(subTypeBaseTypeIdVariant.toULongLong()));
    }
    
    QVariant enumBaseTypeIdVariant(sql.value("e_base_type_id"));
    if (!enumBaseTypeIdVariant.isNull())
    {
        BMCL_ASSERT_MSG(!type, "invalid type");
        QSqlQuery enumConstantsQuery;
        enumConstantsQuery.prepare("SELECT name, info, value FROM enum_type_constant WHERE enum_type_id = :enum_type_id");
        enumConstantsQuery.bindValue("enum_type_id", QVariant::fromValue(typeId));
        execQuery(enumConstantsQuery);
        std::vector<std::shared_ptr<EnumConstant>> constants;
        while (enumConstantsQuery.next())
        {
            constants.emplace_back(new EnumConstant(enumConstantsQuery.value("name").toString().toStdString(),
                                   optionStringOf(enumConstantsQuery.value("info")),
                                   enumConstantsQuery.value("value").toString().toStdString()));
        }
        type = std::make_shared<EnumType>(nameOptional, _namespace, infoOptional,
                                          ensureTypeLoaded(enumBaseTypeIdVariant.toULongLong()), std::move(constants));
    }
    
    QVariant arrayBaseTypeIdVariant(sql.value("ar_base_type_id"));
    if (!arrayBaseTypeIdVariant.isNull())
    {
        BMCL_ASSERT_MSG(!type, "invalid type");
        type = std::make_shared<ArrayType>(nameOptional, _namespace, infoOptional,
                                           ensureTypeLoaded(arrayBaseTypeIdVariant.toULongLong()),
                                           sql.value("min_length").toULongLong(), sql.value("max_length").toULongLong());
    }
    
    QVariant structTypeIdVariant(sql.value("str_type_id"));
    if (!structTypeIdVariant.isNull())
    {
        BMCL_ASSERT_MSG(!type, "invalid type");
        QSqlQuery structFieldsQuery;
        structFieldsQuery.prepare("SELECT name, type_id, unit_id, info FROM struct_type_field WHERE struct_type_id = :struct_type_id ORDER BY field_index ASC");
        structFieldsQuery.bindValue(":struct_type_id", structTypeIdVariant);
        execQuery(structFieldsQuery);
        std::vector<std::shared_ptr<StructField>> fields;
        while (structFieldsQuery.next())
        {
            QVariant unit(structFieldsQuery.value("unit_id"));
            fields.emplace_back(std::make_shared<StructField>(structFieldsQuery.value("name").toString().toStdString(),
                                optionStringOf(structFieldsQuery.value("info")),
                                ensureTypeLoaded(structFieldsQuery.value("type_id").toULongLong()),
                                unit.isNull() ? std::shared_ptr<Unit>() : _unitById.at(unit.toULongLong())));
        }
        BMCL_ASSERT_MSG(fields.size() > 0, "struct must not be empty");
        type = std::make_shared<StructType>(nameOptional, _namespace, infoOptional, std::move(fields));
    }
    
    BMCL_ASSERT_MSG(type, QString("cant't load type %1").arg(typeId).toStdString().c_str());
    _typeById.emplace(typeId, type);
    _namespace->types().emplace_back(type);
    return type;
}

QSqlQuery Sqlite3RegistryProvider::runQuery(const char *sql)
{
    QSqlQuery query(sql, _db);
    return execQuery(query);
}

QSqlQuery Sqlite3RegistryProvider::execQuery(QSqlQuery &query)
{
    if (!query.exec())
    {
        bmcl::panic(QString("%1:%2 %3:%4 for query '%5'").arg(__FILE__).arg(__LINE__).arg(query.lastError().number())
            .arg(query.lastError().text()).arg(query.lastQuery()).toStdString().c_str());
    }
    return query;
}

Option<std::string> Sqlite3RegistryProvider::optionStringOf(QVariant variant)
{
    return variant.isNull() ? Option<std::string>() : Option<std::string>(variant.toString().toStdString());
}

std::shared_ptr<Component> Sqlite3RegistryProvider::loadComponent(uint_fast64_t componentId)
{
    QSqlQuery componentQuery;
    componentQuery.prepare("SELECT namespace_id, name, base_type_id, info FROM component WHERE id = :id");
    componentQuery.bindValue(":id", QVariant::fromValue(componentId));
    execQuery(componentQuery);
    if (!componentQuery.next())
    {
        bmcl::panic(QString("SQL select error, component '%1' not found").arg(componentId).toStdString().c_str());
    }
    auto& _namespace = _namespaceById.at(componentQuery.value("namespace_id").toULongLong());
    QVariant baseTypeId(componentQuery.value("base_type_id"));

    std::shared_ptr<Component> component = std::make_shared<Component>(componentQuery.value("name").toString().toStdString(), _namespace,
                                         baseTypeId.isNull() ? std::shared_ptr<Type>() : ensureTypeLoaded(baseTypeId.toULongLong()),
                                            optionStringOf(componentQuery.value("info")));
    QSqlQuery subComponentsQuery;
    subComponentsQuery.prepare("SELECT sub_component_id FROM sub_component WHERE component_id = :component_id ORDER BY sub_component_id ASC");
    subComponentsQuery.bindValue(":component_id", QVariant::fromValue(componentId));
    execQuery(subComponentsQuery);
    while (subComponentsQuery.next())
    {
        component->subComponents().emplace_back(ensureComponentLoaded(subComponentsQuery.value("sub_component_id").toULongLong()));
    }

    QSqlQuery commandsQuery;
    commandsQuery.prepare("SELECT id, name, command_id, info FROM command WHERE component_id = :component_id");
    commandsQuery.bindValue(":component_id", QVariant::fromValue(componentId));
    execQuery(commandsQuery);
    while (commandsQuery.next())
    {
        uint_fast64_t commandId(commandsQuery.value("id").toULongLong());
        QSqlQuery argumentsQuery;
        argumentsQuery.prepare("SELECT name, type_id, unit_id, info FROM command_argument WHERE command_id = :command_id ORDER BY argument_index ASC");
        argumentsQuery.bindValue(":command_id", QVariant::fromValue(commandId));
        execQuery(argumentsQuery);
        std::vector<std::shared_ptr<CommandArgument>> arguments;
        while (argumentsQuery.next())
        {
            QVariant unitId(argumentsQuery.value("unit_id"));
            arguments.emplace_back(std::make_shared<CommandArgument>(argumentsQuery.value("name").toString().toStdString(),
                                                       ensureTypeLoaded(argumentsQuery.value("type_id").toULongLong()),
                                                       unitId.isNull() ? std::weak_ptr<Unit>() : _unitById.at(unitId.toULongLong()),
                                                       optionStringOf(argumentsQuery.value("info"))));
        }
        component->commands().emplace_back(std::make_shared<Command>(commandsQuery.value("name").toString().toStdString(),
                                                                     commandsQuery.value("command_id").toUInt(),
                                                                     component, std::move(arguments)));
    }

    QSqlQuery messagesQuery;
    messagesQuery.prepare("SELECT m.id AS id, m.name AS name, m.message_id AS message_id, m.info AS info, s.message_id AS s_message_id,"
                                  " e.message_id AS e_message_id, d.message_id AS d_message_id"
                                  " FROM message AS m LEFT JOIN status_message AS s ON s.message_id = m.id LEFT JOIN event_message AS e ON e.message_id = m.id"
                                  " LEFT JOIN dynamic_status_message AS d ON d.message_id = m.id WHERE component_id = :component_id");
    messagesQuery.bindValue(":component_id", QVariant::fromValue(componentId));
    execQuery(messagesQuery);
    while (messagesQuery.next())
    {
        uint_fast64_t id(messagesQuery.value("id").toULongLong());
        std::string name(messagesQuery.value("name").toString().toStdString());
        uint_fast32_t messageId(messagesQuery.value("message_id").toUInt());
        std::shared_ptr<Message> message;
        std::vector<std::shared_ptr<MessageParameter>> parameters;
        QSqlQuery parametersQuery;

        parametersQuery.prepare("SELECT name FROM message_parameter WHERE message_id = :message_id ORDER BY parameter_index ASC");
        parametersQuery.bindValue(":message_id", QVariant::fromValue(id));
        execQuery(parametersQuery);
        while (parametersQuery.next())
        {
            parameters.emplace_back(std::make_shared<MessageParameter>(parametersQuery.value("name").toString().toStdString()));
        }

        if (!messagesQuery.value("s_message_id").isNull())
        {
            message = std::make_shared<StatusMessage>(name, messageId, component, std::move(parameters));
        }
        if (!messagesQuery.value("e_message_id").isNull())
        {
            BMCL_ASSERT_MSG(message == nullptr, "invalid message");
            message = std::make_shared<EventMessage>(name, messageId, component, std::move(parameters));
        }
        if (!messagesQuery.value("d_message_id").isNull())
        {
            BMCL_ASSERT_MSG(message == nullptr, "invalid message");
            message = std::make_shared<DynamicStatusMessage>(name, messageId, component, std::move(parameters));
        }
        BMCL_ASSERT_MSG(message, QString("Can't create message %1").arg(id).toStdString().c_str());
        component->messages().emplace_back(message);
    }
    _componentById.emplace(componentId, component);
    _namespace->components().emplace_back(component);
    return component;
}
}
}
