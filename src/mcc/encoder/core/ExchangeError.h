/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#pragma once

namespace mcc {
namespace encoder {
namespace core {

enum class ExchangeError
{
    PacketCantSendEmpty,
    PacketCantSend,
    DecodePacketNotEnoughDataForPacket,
    PacketCantReceive,
    EncodeNotImplemented,
    EncodeEmptyDataNotAllowed,
    DecodeNotImplemented,
    DecodeBadDataLength,
    DecodeBadAnswer,

    EncodeUnknownCommand,

    DecodeBadCrc,
    DecodeBadStartSequence,

    DeviceWrongCmd,
    DeviceInvalidNodeNum,
    DeviceInvalidNodeId,
    DeviceInvalidVarNum,
    DeviceInvalidVarVal,
    DeviceOther,
    DeviceUnknownError,
    DecodeCantDecodeDeviceError,
};

static inline bool isResendImpossible(ExchangeError err)
{
    switch (err)
    {
    case ExchangeError::PacketCantSendEmpty:
    case ExchangeError::EncodeNotImplemented:
    case ExchangeError::EncodeUnknownCommand:
    case ExchangeError::EncodeEmptyDataNotAllowed:
    case ExchangeError::DecodeNotImplemented:
    case ExchangeError::DecodeCantDecodeDeviceError:
    case ExchangeError::DeviceWrongCmd:
    case ExchangeError::DeviceInvalidNodeNum:
    case ExchangeError::DeviceInvalidNodeId:
    case ExchangeError::DeviceInvalidVarNum:
    case ExchangeError::DeviceInvalidVarVal:
    case ExchangeError::DeviceOther:
    case ExchangeError::DeviceUnknownError:
    case ExchangeError::DecodeBadDataLength:
        return true;
    default:
    break;
    }
    return false;
}
}
}
}


static inline std::string toString(mcc::encoder::core::ExchangeError value)
{
    using namespace mcc::encoder::core;
    switch (value)
    {
    case ExchangeError::PacketCantSend:                 return "Отправка: Невозможно отправить пакет устройству";
    case ExchangeError::PacketCantReceive:              return "Получение: Невозможно получить пакет от устройства";
    case ExchangeError::EncodeUnknownCommand:           return "Формирование пакета: Неизвестная команда для кодирования";
    case ExchangeError::PacketCantSendEmpty:            return "Отпрвка: нельзя отправить пустой пакет";
    case ExchangeError::DecodePacketNotEnoughDataForPacket:return "Разбор пакета: Недостаточно данных для пакета";
    case ExchangeError::DecodeBadCrc:                   return "Разбор пакета: Некорректная контрольная сумма";
    case ExchangeError::DecodeBadStartSequence:         return "Разбор пакета: Неверная стартовая последовательность";
    case ExchangeError::DecodeBadDataLength:            return "Разбор пакета: Неверная длина данных";
    case ExchangeError::DeviceWrongCmd:                 return "Устройство: Неверная команда";
    case ExchangeError::DeviceInvalidNodeNum:           return "Устройство: Недопустимый номер узла";
    case ExchangeError::DeviceInvalidNodeId:            return "Устройство: Недопустимый идентификатор узла";
    case ExchangeError::DeviceInvalidVarNum:            return "Устройство: Недопустимый номер переменной";
    case ExchangeError::DeviceInvalidVarVal:            return "Устройство: Недопустимое значение переменной";
    case ExchangeError::DeviceOther:                    return "Устройство: Другая ошибка";
    case ExchangeError::DeviceUnknownError:             return "Неизвестная ошибка от устройства";
    case ExchangeError::DecodeCantDecodeDeviceError:    return "Невозможно декодировать ошибку от устройства";
    case ExchangeError::DecodeBadAnswer:                return "Разбор пакета: некорректный тип ответа";
    default:                                            return "Ошибка: " + std::to_string((int)value);
    }
}