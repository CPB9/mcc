/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#pragma once
#include <QString>
#include <QMetaType>

namespace mcc
{
namespace misc
{

struct ProtocolDescription
{
    std::size_t id;
    QString     name;
    QString     info;
    QString     param_info;
    QString     service;
    QString     trait;
};

class ProtocolId
{
public:
    ProtocolId(){}
//     explicit ProtocolId(const std::string& protocolId) : _id(QString::fromStdString(protocolId)){}
//     explicit ProtocolId(const QString& protocolId) : _id(protocolId){}
    ProtocolId(const QString& protocol, std::size_t id) : _id(id), _protocol(protocol){}
    const QString& protocol() const { return _protocol; }
    std::size_t id() const { return _id; }
private:
    std::size_t _id;
    QString     _protocol;
};


}
}


Q_DECLARE_METATYPE(mcc::misc::ProtocolDescription);
Q_DECLARE_METATYPE(mcc::misc::ProtocolId);
