/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#pragma once
#include <functional>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QSqlQuery>
#include <QDir>
#include <QFileInfoList>

namespace mcc {
namespace core {
namespace db {

class FirmwareLoader
{
public:
    static bool load(const QString& traitsDirPath, QSqlQuery query);

private:
    static void processKind(const QString& kind, std::function<void(QSqlQuery&, const QJsonObject&, const QString& file)> fn, const QFileInfoList& files, QSqlQuery query);
    static void processDevice(QSqlQuery& query, const QJsonObject& device, const QString& file);
    static void processDeviceKind(QSqlQuery& query, const QJsonObject& kind, const QString& file);
    static void processFirmware(QSqlQuery& query, const QJsonObject& firmware, const QString& file);
    static void processTrait(QSqlQuery& query, const QJsonObject& trait, const QString& file);
    static void processProtocol(QSqlQuery& query, const QJsonObject& protocol, const QString& file);
    static void processService(QSqlQuery& query, const QJsonObject& service, const QString& file);
};

}
}
}