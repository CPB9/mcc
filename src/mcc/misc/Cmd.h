/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#pragma once
#include <cstdint>
#include <QString>
#include <QStringList>
#include <QMetaType>
#include "mcc/misc/NetVariant.h"


namespace mcc {
namespace misc {

enum class CmdState
{
    Created,
    Registered,
    WaitingForTime,
    RoutedForDelivery,
    WaitingInDeliveryQueue,
    SentToDevice,
    AcknowledgeReceived,
//            Executed,
    Failed,
};

static inline const char* toString(CmdState state)
{
    switch (state)
    {
    case CmdState::Created:               return "Команда создана";
    case CmdState::Registered:            return "Команда зарегистрирована";
    case CmdState::WaitingForTime:        return "Команда ожидает отправки";
    case CmdState::RoutedForDelivery:     return "Команда маршрутизирована";
    case CmdState::WaitingInDeliveryQueue:return "Команда ожидает в очереди";
    case CmdState::SentToDevice:          return "Команда отправлена на устройство";
    case CmdState::AcknowledgeReceived:   return "Получена квитанция на команду";
    case CmdState::Failed:                return "Ошибка при отправке команды";
    }
    Q_ASSERT(false);
    return "Неизвестное состояние обработки команды";
}

typedef uint32_t CmdCollationId;
typedef std::vector<mcc::misc::NetVariant> CmdParams;

class Cmd
{
public:
    Cmd():_collationId(uint32_t(-1)){}
    Cmd(CmdCollationId collationId, const QString& device, const QString& trait, const QString& command, const CmdParams& params = CmdParams())
        :_collationId(collationId), _device(device), _trait(trait), _command(command), _params(params)
    {
    }

    Cmd(const Cmd& other)
        : Cmd(other.collationId(), other.device(), other.trait(), other.command(), other.params())
    {
    }
    CmdCollationId collationId()   const { return _collationId; }
    const QString& device()        const { return _device; };
    const QString& trait()         const { return _trait; }
    const QString& command()       const { return _command; }
    const CmdParams& params()    const { return _params; }
    std::string paramsAsString(const std::string& delimeter) const
    {
        if (_params.empty())
            return std::string();

        std::string r = _params[0].stringify();
        for (std::size_t i = 1; i < _params.size(); ++i)
        {
            r += delimeter + _params[i].stringify();
        }
        return r;
    }

    QString paramsAsQString(const std::string& delimeter) const
    {
        return QString::fromStdString(paramsAsString(delimeter));
    }

    QString name() const
    {
        return QString("%1.%2.%3").arg(_device).arg(_trait).arg(_command);
    }

    static CmdCollationId generateCollationId()
    {
        static CmdCollationId cmdCollationId = 0;
        return ++cmdCollationId;
    }

private:
    CmdCollationId _collationId;
    QString       _device;
    QString       _trait;
    QString       _command;
    CmdParams     _params;
};
}
}

Q_DECLARE_METATYPE(mcc::misc::CmdCollationId)
Q_DECLARE_METATYPE(mcc::misc::CmdState);
Q_DECLARE_METATYPE(mcc::misc::Cmd)