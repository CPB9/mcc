/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include <QSqlRecord>
#include <QSqlField>
#include <QSqlError>
#include <QVariant>

#include "mcc/messages/Firmware.h"
#include "mcc/messages/MessageSender.h"
#include "mcc/core/db/DbHandle.h"
#include "mcc/core/db/Sql.h"


namespace mcc {
namespace core {
namespace db {
namespace queries
{

bmcl::Option<SqlError> getFirmware(QSqlQuery* query, const std::string& firmware, mcc::misc::FirmwareDescription* description)
{
    query->prepare("select id, info, registered, source from firmware where name = :name;");
    query->bindValue(":name", QString::fromStdString(firmware));
    auto r = execSelect(query);
    printIfErr(r);
    if (r.isSome())
        return r.take();

    if (!query->next())
        return SqlError("not found");

    QSqlRecord record = query->record();
    description->_id = record.value("id").toUInt();
    description->_info = record.value("info").toString();
    description->_registered = record.value("registered").toString();
    description->_source = record.value("source").toString();
    description->_name = QString::fromStdString(firmware);

    return bmcl::None;
}

bmcl::Option<SqlError> getTraits(QSqlQuery* query, std::size_t firmware_id, mcc::misc::TraitDescriptionList* traits)
{
    query->prepare("select trait.id             as id           \
                            , trait.parent_id      as parent_id    \
                            , trait.unique_name    as unique_name  \
                            , trait.name           as name         \
                            , trait.bit_size       as bit_size     \
                            , trait.kind           as kind         \
                            , trait.info           as info         \
                            , guid.name            as guid_name    \
                        from trait                           \
                            , guid                            \
                        where trait.guid_id=guid.id           \
                        and trait.firmware_id= :firmware_id \
                            ;");
    query->bindValue(":firmware_id", (qulonglong)firmware_id);
    auto r = execSelect(query);
    if (r.isSome())
    {
        printIfErr(r);
        return r;
    }

    while (query->next())
    {
        QSqlRecord r = query->record();
        mcc::misc::TraitDescription trait;
        //trait->_guid        = r.value("guid_name"  ).toString();
        trait._id          = r.value("id"         ).toUInt();
        trait._parentId    = r.value("parent_id"  ).toUInt();
        trait._unique_name = r.value("unique_name").toString();
        trait._name        = r.value("name"       ).toString();
        //trait._bit_size    = r.value("bit_size"   ).toUInt();
        //trait._kind        = r.value("kind"       ).toString();
        trait._info        = r.value("info"       ).toString();
        traits->push_back(trait);
    }

    query->clear();
    return bmcl::None;
}

bmcl::Option<SqlError>  getFields(QSqlQuery* query, std::size_t trait_id, mcc::misc::TmParamDescriptionList* fields)
{
    query->prepare("select field.id                                \
                            , field.number                         \
                            , field.name                           \
                            , field.unit                           \
                            , field.info                           \
                            , field.properties                     \
                            , field.value_min                      \
                            , field.value_max                      \
                            , type.name           as type_name     \
                        from trait       t                         \
                            , trait_field field                    \
                            , trait       type                     \
                        where t.id = :trait_id                     \
                        and t.id = field.trait_id                \
                        and field.type_id = type.id              \
                    ;");
    query->bindValue(":trait_id", (qulonglong)trait_id);
    auto r = execSelect(query);
    if (r.isSome())
    {
        printIfErr(r);
        return r;
    }

    while (query->next())
    {
        QSqlRecord r = query->record();
        mcc::misc::TmParamDescription field;
        field._id           = r.value("id"           ).toUInt();
        field._number       = r.value("number"       ).toUInt();
        field._name         = r.value("name"         ).toString();
        field._type         = r.value("type_name"    ).toString();
        field._unit         = r.value("unit"         ).toString();
        field._info         = r.value("info"         ).toString();
        field._properties   = r.value("properties"   ).toString();
        if (!r.value("value_min").isNull())
            field._min = r.value("value_min").toDouble();
        if (!r.value("value_max").isNull())
            field._max = r.value("value_max").toDouble();
        fields->push_back(field);
    }

    query->clear();
    return bmcl::None;
}

bmcl::Option<SqlError> getMethods(QSqlQuery* query, std::size_t trait_id, mcc::misc::CmdDescriptionList* fields)
{
    query->prepare("select trait_method.id                  \
                            , trait_method.number              \
                            , trait_method.name                \
                            , trait_method.info                \
                        from trait                            \
                            , trait_method                     \
                        where trait.id = :trait_id             \
                        and trait.id = trait_method.trait_id \
                            ;");
    query->bindValue(":trait_id", (qulonglong)trait_id);
    auto r = execSelect(query);
    if (r.isSome())
    {
        printIfErr(r);
        return r;
    }

    while (query->next())
    {
        QSqlRecord r = query->record();
        mcc::misc::CmdDescription field;
        field._id       = r.value("id"       ).toUInt();
        field._number   = r.value("number"   ).toUInt();
        field._name     = r.value("name"     ).toString();
        field._info     = r.value("info"     ).toString();
        fields->push_back(field);
    }

    query->clear();
    return bmcl::None;
}

bmcl::Option<SqlError> getMethodArgs(QSqlQuery* query, std::size_t method_id, mcc::misc::CmdParamDescriptionList* args)
{
    query->prepare("select trait_method_arg.id                         \
                            , trait_method_arg.number                  \
                            , trait_method_arg.name                    \
                            , trait_method_arg.info                    \
                            , trait_method_arg.unit                    \
                            , trait_method_arg.value_min               \
                            , trait_method_arg.value_max               \
                            , trait.name               as trait_name   \
                        from trait                                     \
                            , trait_method_arg                         \
                        where trait_method_arg.method_id = :method_id  \
                        and trait_method_arg.type_id   = trait.id    \
                    ;");

    query->bindValue(":method_id", (qulonglong)method_id);
    auto r = execSelect(query);
    if (r.isSome())
    {
        printIfErr(r);
        return r;
    }

    while (query->next())
    {
        QSqlRecord r = query->record();

        mcc::misc::CmdParamDescription arg;
        arg._id         = r.value("id"           ).toUInt();
        arg._number     = r.value("number"       ).toUInt();
        arg._name       = r.value("name"         ).toString();
        arg._info       = r.value("info"         ).toString();
        arg._unit       = r.value("unit"         ).toString();
        arg._type       = r.value("trait_name"   ).toString();
        if (!r.value("value_min").isNull())
            arg._min = r.value("value_min").toDouble();
        if (!r.value("value_max").isNull())
            arg._max = r.value("value_max").toDouble();
        args->push_back(arg);
    }

    query->clear();
    return bmcl::None;
}

bmcl::Option<SqlError> getFirmwareDescription(QSqlQuery* query, const std::string& firmware, const mcc::misc::FirmwareDescriptionPtr& description)
{
    auto r = getFirmware(query, firmware, description.get());
    if (r.isSome())
        return r.take();

    auto& traits = description->_traits;
    auto traitsErr = getTraits(query, description->_id, &traits);
    if (traitsErr.isSome())
        return traitsErr.take();

    for (auto& trait: traits)
    {
        auto r = getFields(query, trait._id, &trait._tmParams);
        if (r.isSome())
            return r.take();

        r = getMethods(query, trait._id, &trait._cmds);
        if (r.isSome())
            return r.take();

        for (auto& method : trait._cmds)
        {
            r = getMethodArgs(query, method._id, &method._args);
            if (r.isSome())
                return r;
        }
    }
    return bmcl::None;
}

void execute(DbHandle* db, const mcc::messages::MessageSender& sender, std::unique_ptr<mcc::messages::FirmwareDescription_Request>&& request)
{
    mcc::misc::FirmwareDescriptionPtr description = std::make_shared<mcc::misc::FirmwareDescription>();
    QSqlQuery query = db->getQueryHandle();
    auto r = getFirmwareDescription(&query, request->firmware(), description);
    if (r.isSome())
    {
        description->_id = std::size_t(-1);
        sender->respond<mcc::messages::FirmwareDescription_Response>(request.get(), description, "can't load firmware description: " + r.take().err.toStdString());
        return;
    }

    sender->respond<mcc::messages::FirmwareDescription_Response>(request.get(), description);
}
}
}
}
}