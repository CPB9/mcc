/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include <assert.h>
#include "mcc/misc/TimeUtils.h"
#include "mcc/core/db/Sql.h"
#include "mcc/core/db/FirmwareLoader.h"

namespace mcc {
namespace core {
namespace db {

void FirmwareLoader::processKind(const QString& kind, std::function<void(QSqlQuery&, const QJsonObject&, const QString& file)> fn, const QFileInfoList& files, QSqlQuery query)
{
    for (const QFileInfo& file : files)
    {

        if (!QFile::exists(file.filePath()))
        {
            assert(false);
            return;
        }

        QFile firmware(file.filePath());
        if (!firmware.open(QIODevice::ReadOnly | QIODevice::Text))
        {
            assert(false);
            return;
        }

        QJsonParseError error;
        QJsonDocument json = QJsonDocument::fromJson(firmware.readAll(), &error);
        if (json.isNull())
        {
            assert(false);
            return;
        }

        if (!json.isObject())
        {
            return;
        }

        for (const QJsonValue& i : json.object().value(kind).toArray())
        {
            fn(query, i.toObject(), file.fileName());
        }
    }
}

SqlResult insertDeviceKind(QSqlQuery& query, const QString& name, const QString& info)
{
    QString time = QString::fromStdString(mcc::misc::currentDateTime());
    query.prepare("insert into device_kind (name, info) values (:name, :info)");
    query.bindValue(":name", name);
    query.bindValue(":info", info);
    return execInsert(query);
}

SqlResult insertDevice(QSqlQuery& query, const QString& name, const QString& info, const QString& kind, const QString& firmware)
{
    QString time = QString::fromStdString(mcc::misc::currentDateTime());
    query.prepare("insert into device (name, info, registered, updated, kind_id, firmware_id) \
                               values (:name, :info, :registered, :updated, (select id from device_kind where name=:kind), (select id from firmware where name=:firmware))");
    query.bindValue(":name", name);
    query.bindValue(":info", info);
    query.bindValue(":kind", kind);
    query.bindValue(":firmware", firmware);
    query.bindValue(":registered", time);
    query.bindValue(":updated", time);
    return execInsert(query);
}

SqlResult insertDeviceProtocol(QSqlQuery& query, const QVariant& device_id, const QString& protocol, const QString& value)
{
    query.prepare("insert into device_protocol (device_id, protocol_value, protocol_id) values (:device_id, :value, (select id from protocol where name=:protocol))");
    query.bindValue(":device_id", device_id);
    query.bindValue(":value", value);
    query.bindValue(":protocol", protocol);
    return execInsert(query);
}

SqlResult insertFirmware(QSqlQuery& query, const QString& name, const QString& info, const QString& file)
{
    query.prepare("insert into firmware (name, info, source, registered) values (:name, :info, :source, :registered)");
    query.bindValue(":name", name);
    query.bindValue(":info", info);
    query.bindValue(":source", file);
    query.bindValue(":registered", QString::fromStdString(mcc::misc::currentDateTime()));
    return execInsert(query);
}

SqlResult insertGuid(QSqlQuery& query, const QString& name)
{
    query.prepare("insert into guid (name) values (:name)");
    query.bindValue(":name", name);
    return execInsert(query);
}

SqlResult insertTraitInfo(QSqlQuery& query, const QVariant& firmware_id, const QVariant& parent_id, const QString& full_name, const QString& name, const QVariant& guid_id, const QVariant& info, const QVariant& bit_size, const QVariant& kind)
{
    query.prepare("insert into trait (firmware_id, parent_id, name, unique_name, number, info, bit_size, kind, guid_id) \
                                values (:firmware_id, :parent_id, :name, :unique_name, :number, :info, :bit_size, :kind, :guid_id)");
    query.bindValue(":name", name);
    query.bindValue(":firmware_id", firmware_id);
    query.bindValue(":unique_name", full_name);
    query.bindValue(":parent_id", parent_id);
    query.bindValue(":number", guid_id);
    query.bindValue(":info", info);
    query.bindValue(":bit_size", bit_size);
    query.bindValue(":kind", kind);
    query.bindValue(":guid_id", guid_id);
    return execInsert(query);
}

SqlResult insertCommand(QSqlQuery& query, const QVariant& trait_id, const QString& name, const QString& info, const QVariant& name_id)
{
    query.prepare("insert into trait_method (trait_id, number, name, info) values (:trait_id, :name_id, :name, :info)");
    query.bindValue(":trait_id", trait_id);
    query.bindValue(":name_id", name_id);
    query.bindValue(":name", name);
    query.bindValue(":info", info);
    return execInsert(query);
}

SqlResult insertCommandParam(QSqlQuery& query, const QVariant& command_id, int param_name_id, const QJsonObject& param)
{
    query.prepare("insert into trait_method_arg (method_id, number, name, info, unit, value_min, value_max, type_id) values (:command_id, :name_id, :name, :info, :unit, :min, :max, (select id from trait where name=:type))");
    query.bindValue(":command_id", command_id);
    query.bindValue(":name_id", param_name_id);
    query.bindValue(":name", param.value("name").toVariant());
    query.bindValue(":info", param.value("info").toVariant());
    query.bindValue(":unit", param.value("unit").toVariant());
    query.bindValue(":type", param.value("type").toVariant());
    query.bindValue(":min", param.value("min").toVariant());
    query.bindValue(":max", param.value("max").toVariant());
    return execInsert(query);
}

SqlResult insertField(QSqlQuery& query, const QVariant& trait_id, const int field_index, const QJsonObject& field)
{
    query.prepare("insert into trait_field (trait_id, number, name, info, unit, value_min, value_max, type_id) values (:trait_id, :name_id, :name, :info, :unit, :min, :max, (select id from trait where name=:type))");
    query.bindValue(":trait_id", trait_id);
    query.bindValue(":name_id", field_index);
    query.bindValue(":name", field.value("name").toVariant());
    query.bindValue(":info", field.value("info").toVariant());
    query.bindValue(":unit", field.value("unit").toVariant());
    query.bindValue(":type", field.value("type").toVariant());
    query.bindValue(":min", field.value("min").toVariant());
    query.bindValue(":max", field.value("max").toVariant());
    return execInsert(query);
}

SqlResult insertProtocol(QSqlQuery& query, const QString& name, const QString& info, const QString& param_info, const QString& service, const QString& trait)
{
    query.prepare("insert into protocol (name, info, param_info, service_id, trait_id) values (:name, :info, :param_info, (select id from service where name=:service), (select id from trait where name=:trait));");
    query.bindValue(":name", name);
    query.bindValue(":info", info);
    query.bindValue(":param_info", param_info);
    query.bindValue(":trait", trait);
    query.bindValue(":service", service);
    return execInsert(query);
}

SqlResult insertService(QSqlQuery& query, const QString& name, const QString& info)
{
    query.prepare("insert into service (name, info) values (:name, :info);");
    query.bindValue(":name", name);
    query.bindValue(":info", info);
    return execInsert(query);
}

SqlResult processCommand(QSqlQuery& query, const QVariant& trait_id, const QJsonObject& command)
{
    QString name = command.value("name").toString();
    QString info = command.value("info").toVariant().toString();
    QVariant name_id = command.value("name_id").toVariant();

    auto c = insertCommand(query, trait_id, name, info, name_id);
    if (c.isErr())
        return c.takeErr();
    QVariant command_id = c.unwrap();

    int param_name_id = 0;
    for (const QJsonValue& i : command.value("params").toArray())
    {
        auto p = insertCommandParam(query, command_id, param_name_id, i.toObject());
        if (p.isErr())
            return c.takeErr();
        ++param_name_id;
    }

    return command_id;
}

bmcl::Option<SqlError> processSubTrait(QSqlQuery& query, const QVariant& firmware_id, const QJsonObject& trait, const QVariant& parent_id, const QString& full_parent);

bmcl::Option<SqlError> processTraitElements(QSqlQuery& query, const QString& full_name, const QJsonArray& traits, const QJsonArray& commands, const QJsonArray& fields, const QVariant& trait_id, const QVariant& firmware_id)
{
    for (const QJsonValue& i : traits)
    {
        auto r = processSubTrait(query, firmware_id, i.toObject(), trait_id, full_name);
        if (r.isSome())
            return r.take();
    }

    for (const QJsonValue& i : commands)
    {
        auto r = processCommand(query, trait_id, i.toObject());
        if (r.isErr())
            return r.takeErr();
    }

    int field_name_id = 0;
    for (const QJsonValue& i : fields)
    {
        auto f = insertField(query, trait_id, field_name_id, i.toObject());
        if (f.isErr())
            return f.takeErr();
        ++field_name_id;
    }
    return bmcl::None;
}

bmcl::Option<SqlError> processSubTrait(QSqlQuery& query, const QVariant& firmware_id, const QJsonObject& trait, const QVariant& parent_id, const QString& full_parent)
{
    QString name(trait.value("name").toVariant().toString());
    QString fullName;
    if (!full_parent.isEmpty())
        fullName = QString("%1.%2").arg(full_parent).arg(name);
    else
        fullName = name;

    if (trait.value("kind").toString() == "array" && trait.contains("size"))
    {
        std::size_t size = trait.value("size").toInt();
        for (std::size_t i = 0; i < size; ++i)
        {
            QString elem = QString("%1%2").arg(name).arg(i);
            QString fullElem = QString("%1%2").arg(fullName).arg(i);
            auto g = insertGuid(query, fullElem);
            if (g.isErr())
                return g.unwrapErr();
            QVariant guid_id = g.unwrap();
            QString info = QString("%1 %2").arg(trait.value("info").toString()).arg(i);
            auto t = insertTraitInfo(query, firmware_id, parent_id, fullElem, elem, guid_id, info, trait.value("bit_size").toVariant(), "struct");
            if (t.isErr())
                return t.takeErr();
            QVariant trait_id = t.unwrap();
            auto r = processTraitElements(query, fullElem, trait.value("traits").toArray(), trait.value("commands").toArray(), trait.value("fields").toArray(), trait_id, firmware_id);
            if (r.isSome())
                return r.take();
        }
    }
    else
    {
        auto g = insertGuid(query, fullName);
        if (g.isErr())
            return g.unwrapErr();
        QVariant guid_id = g.unwrap();
        auto t = insertTraitInfo(query, firmware_id, parent_id, fullName, name, guid_id, trait.value("info").toVariant(), trait.value("bit_size").toVariant(), trait.value("kind").toVariant());
        if (t.isErr())
            return t.takeErr();
        QVariant trait_id = t.unwrap();
        auto r = processTraitElements(query, fullName, trait.value("traits").toArray(), trait.value("commands").toArray(), trait.value("fields").toArray(), trait_id, firmware_id);
        if (r.isSome())
            return r.take();
    }

    return bmcl::None;
}

void FirmwareLoader::processDeviceKind(QSqlQuery& query, const QJsonObject& kind, const QString& file)
{
    QString name(kind.value("name").toVariant().toString());
    QString info(kind.value("info").toVariant().toString());

    auto r = insertDeviceKind(query, name, info);
    printIfErr(r);
}

void FirmwareLoader::processDevice(QSqlQuery& query, const QJsonObject& device, const QString& file)
{
    QString name(device.value("name").toVariant().toString());
    QString info(device.value("info").toVariant().toString());
    QString kind(device.value("kind").toVariant().toString());
    QString firmware(device.value("firmware").toVariant().toString());

    auto d = insertDevice(query, name, info, kind, firmware);
    if (d.isErr())
    {
        print(d.takeErr());
        return;
    }
    QVariant device_id = d.unwrap();

    auto ps = device.value("protocols").toObject();
    for (auto p = ps.begin(); p != ps.end(); ++p)
    {
        auto r = insertDeviceProtocol(query, device_id, p.key(), QJsonValue(p.value()).toVariant().toString());
        printIfErr(r);
    }
}

void FirmwareLoader::processFirmware(QSqlQuery& query, const QJsonObject& firmware, const QString& file)
{
    QString name(firmware.value("name").toVariant().toString());
    QString info(firmware.value("info").toVariant().toString());

    auto f = insertFirmware(query, name, info, file);
    if (f.isErr())
    {
        print(f.takeErr());
        return;
    }
    QVariant firmware_id = f.unwrap();

    for (const QJsonValue& i : firmware.value("traits").toArray())
    {
        auto r = processSubTrait(query, firmware_id, i.toObject(), QString(), QString());
        printIfErr(r);
    }
}

void FirmwareLoader::processTrait(QSqlQuery& query, const QJsonObject& trait, const QString& file)
{
    auto r = processSubTrait(query, QVariant(), trait, QString(), QString());
    printIfErr(r);
}

void FirmwareLoader::processProtocol(QSqlQuery& query, const QJsonObject& protocol, const QString& file)
{
    QString name(protocol.value("name").toVariant().toString());
    QString info(protocol.value("info").toVariant().toString());
    QString trait(protocol.value("trait").toVariant().toString());
    QString service(protocol.value("service").toVariant().toString());
    QString param_info(protocol.value("param_info").toVariant().toString());

    auto r = insertProtocol(query, name, info, param_info, service, trait);
    printIfErr(r);
}

void FirmwareLoader::processService(QSqlQuery& query, const QJsonObject& service, const QString& file)
{
    QString name(service.value("name").toVariant().toString());
    QString info(service.value("info").toVariant().toString());
    printIfErr(insertService(query, name, info));
}

bool FirmwareLoader::load(const QString& traitsDirPath, QSqlQuery query)
{
    QDir traitsDir(traitsDirPath, "*.json", QDir::Name, QDir::Files);
    QFileInfoList files = traitsDir.entryInfoList();
    if (files.empty())
        return false;

    processKind("services", processService, files, query);
    processKind("traits", processTrait, files, query);
    processKind("firmwares", processFirmware, files, query);
    processKind("protocols", processProtocol, files, query);
    processKind("device_kinds", processDeviceKind, files, query);
    processKind("devices", processDevice, files, query);

    return true;
}

}
}
}
