/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include <QSqlRecord>
#include <QSqlField>
#include <QSqlError>
#include <QSqlQuery>
#include <QVariant>
#include "bmcl/Result.h"

#include "mcc/messages/Device.h"
#include "mcc/messages/Firmware.h"
#include "mcc/messages/MessageSender.h"
#include "mcc/core/db/DbHandle.h"
#include "mcc/core/db/Sql.h"

#include "mcc/misc/TraitSort.h"

#define get_or_exit_on_error(response, expression)                                                                     \
    ;                                                                                                                  \
    auto temp_##response = expression;                                                                                 \
    if (temp_##response.isErr())                                                                                       \
        return temp_##response.unwrapErr();                                                                            \
    auto response = temp_##response.unwrap();

#define exit_on_error(expression)                                                                                      \
    ;                                                                                                                  \
    auto temp_##response = expression;                                                                                 \
    if (temp_##response.isSome())                                                                                      \
        return temp_##response.take();                                                                                 \


namespace mcc {
namespace core {
namespace db {
namespace queries {

struct RegistrationResult
{
    std::size_t firmwareId;
};

static bmcl::Option<QVariant> selectFirmware(QSqlQuery& query, const QString& name)
{
    query.prepare("select id from firmware where name=:name;");
    query.bindValue(":name", name);
    auto r = execSelect(query);
    if (r.isSome())
        return bmcl::None;
    while (query.next())
    {
        return query.record().value(0);
    }
    return bmcl::None;
}

static SqlResult insertFirmware(QSqlQuery& query, const QString& name, const QString& info, const QString& source)
{
    query.prepare("insert into firmware (name, info, registered, source) values (:name, :info, :registered, :source)");
    query.bindValue(":name", name);
    query.bindValue(":info", info);
    query.bindValue(":source", source);
    query.bindValue(":registered", QString::fromStdString(mcc::misc::currentDateTime()));
    return execInsert(query);
}

static SqlResult insertField(QSqlQuery& query, std::size_t trait_id, std::size_t field_index, const mcc::misc::TmParamDescription& var)
{
    query.prepare("insert into trait_field (trait_id, number, name, info, unit, properties, value_min, value_max, type_id) values (:trait_id, :name_id, :name, :info, :unit, :properties, :min, :max, (select id from trait where name=:type))");
    query.bindValue(":trait_id", (qulonglong)trait_id);
    query.bindValue(":name_id", (qulonglong)field_index);
    query.bindValue(":name", var._name);
    query.bindValue(":info", var._info);
    query.bindValue(":type", var._type);
    query.bindValue(":unit", var._unit);
    QVariant min = var._min.isSome() ? QVariant(var._min.unwrap()) : QVariant();
    QVariant max = var._max.isSome() ? QVariant(var._max.unwrap()) : QVariant();
    query.bindValue(":min", min);
    query.bindValue(":max", max);
    query.bindValue(":properties", var._properties);
    return execInsert(query);
}

static SqlResult insertGuid(QSqlQuery& query, const QString& name)
{
    query.prepare("insert into guid (name) values (:name)");
    query.bindValue(":name", name);
    return execInsert(query);
}

static SqlResult insertTrait(QSqlQuery& query, std::size_t firmware_id, std::size_t quid_id, const misc::TraitDescription& trait)
{
    query.prepare("insert into trait (name, unique_name, firmware_id, number, info, bit_size, kind, guid_id, parent_id) \
                                values (:name, :unique_name, :firmware_id, :number, :info, :bit_size, :kind, :guid_id, (select id from trait where firmware_id = :firmware_id and name = :parent))");
    query.bindValue(":firmware_id", (qulonglong)firmware_id);
    query.bindValue(":parent", (trait._parentId == trait._id ? QString() : QString::number(trait._parentId)));
    query.bindValue(":name", trait._name);
    query.bindValue(":unique_name", trait._unique_name);
    query.bindValue(":number", (qulonglong)trait._id);
    query.bindValue(":info", trait._info);
    query.bindValue(":kind", "interface");
    query.bindValue(":guid_id", (qulonglong)quid_id);
    return execInsert(query);
}

static bmcl::Result<std::size_t, SqlError>  insert(QSqlQuery& query, const mcc::misc::FirmwareDescriptionPtr& firmware)
{
    auto fs = selectFirmware(query, firmware->_name);
    if (fs.isSome())
        return fs.take().toULongLong();

    get_or_exit_on_error(firmware_id, insertFirmware(query, firmware->_name, firmware->_info, firmware->_source));
    auto levels = sortTraits(firmware->_traits);
    for (const auto& i: levels)
    {
        const auto& trait = firmware->_traits[i.index];
        get_or_exit_on_error(guid_id, insertGuid(query, QString("%1.%2").arg(firmware_id.toUInt()).arg(trait._id)));
        get_or_exit_on_error(trait_id, insertTrait(query, firmware_id.toUInt(), guid_id.toUInt(), trait));

        for (std::size_t i = 0; i < std::size_t(trait._tmParams.size()); ++i)
        {
            get_or_exit_on_error(field_id, insertField(query, trait_id.toUInt(), i, trait._tmParams[i]));
        }
    }
    return firmware_id.toULongLong();
}

void execute(DbHandle* db, const mcc::messages::MessageSender& sender, std::unique_ptr<mcc::messages::FirmwareRegister_Request>&& request)
{
    db->transaction();
    QSqlQuery query = db->getQueryHandle();
    auto r = insert(query, request->firmware());
    if (r.isErr())
    {
        db->rollback();
        print(r.takeErr());
        sender->respond<mcc::messages::FirmwareRegister_Response>(request.get(), "Unable to register device. Firmware description is invalid");
        return;
    }

    db->commit();

    sender->send<mcc::messages::FirmwareRegistered>(request->name(), r.unwrap(), true);
    sender->respond<mcc::messages::FirmwareRegister_Response>(request.get(), r.unwrap());
}
}
}
}
}
