/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include <mcc/misc/NetVariant.h>
#include <bmcl/Reader.h>

#include <QString>
#include <QVariant>

#include <string>

namespace mcc {
namespace misc {

NetVariant::NetVariant(const QVariant& value)
{
    switch (value.type())
    {
    case QVariant::Bool: init(value.toBool(), NetVariantType::Bool); break;
    case QVariant::Int: init(value.toInt(), NetVariantType::Int); break;
    case QVariant::LongLong: init(value.toInt(), NetVariantType::Int); break;
    case QVariant::UInt: init(value.toUInt(), NetVariantType::Uint); break;
    case QVariant::ULongLong: init(value.toUInt(), NetVariantType::Uint); break;
    case QVariant::Double: init(value.toDouble(), NetVariantType::Double); break;
    case QVariant::String: init(value.toString(), NetVariantType::QString); break;
    default:
        assert(false);
        _type = NetVariantType::None;
    }
}

void NetVariant::construct(const NetVariant& other)
{
    switch (other._type) {
    case NetVariantType::Bool:
        *cast<bool>() = *other.cast<bool>();
        break;
    case NetVariantType::Int:
        *cast<std::int64_t>() = *other.cast<std::int64_t>();
        break;
    case NetVariantType::Uint:
        *cast<std::uint64_t>() = *other.cast<std::uint64_t>();
        break;
    case NetVariantType::Float:
        *cast<float>() = *other.cast<float>();
        break;
    case NetVariantType::Double:
        *cast<double>() = *other.cast<double>();
        break;
    case NetVariantType::String:
        new (cast<std::string>()) std::string(*other.cast<std::string>());
        break;
    case NetVariantType::QString:
        new (cast<QString>()) QString(*other.cast<QString>());
        break;
    }
    _type = other._type;
}

std::string NetVariant::stringify() const
{
    switch (_type) {
    case NetVariantType::None:
        return std::string();
    case NetVariantType::Bool: {
        bool value = asBool();
        if (value) {
            return "true";
        }
        return "false";
    }
    case NetVariantType::Int:
        return std::to_string(asInt());
    case NetVariantType::Uint:
        return std::to_string(asUint());
    case NetVariantType::Float:
        return std::to_string(asFloat());
    case NetVariantType::Double:
        return std::to_string(asDouble());
    case NetVariantType::String:
        return asString();
    case NetVariantType::QString:
        return asQString().toStdString();
    }
}

QString NetVariant::qstringify() const
{
    switch (_type) {
    case NetVariantType::None:
        return QString();
    case NetVariantType::Bool: {
         bool value = asBool();
        if (value) {
            return "true";
        }
        return "false";
    }
    case NetVariantType::Int:
        return QString::number(asInt());
    case NetVariantType::Uint:
        return QString::number(asUint());
    case NetVariantType::Float:
        return QString::number(asFloat());
    case NetVariantType::Double:
        return QString::number(asDouble());
    case NetVariantType::String:
        return QString::fromStdString(asString());
    case NetVariantType::QString:
        return asQString();
    }
}

void NetVariant::construct(NetVariant&& other)
{
    switch (other._type) {
    case NetVariantType::Bool:
        *cast<bool>() = *other.cast<bool>();
        break;
    case NetVariantType::Int:
        *cast<std::int64_t>() = *other.cast<std::int64_t>();
        break;
    case NetVariantType::Uint:
        *cast<std::uint64_t>() = *other.cast<std::uint64_t>();
        break;
    case NetVariantType::Float:
        *cast<float>() = *other.cast<float>();
        break;
    case NetVariantType::Double:
        *cast<double>() = *other.cast<double>();
        break;
    case NetVariantType::String:
        new (cast<std::string>()) std::string(std::move(*other.cast<std::string>()));
        break;
    case NetVariantType::QString:
        new (cast<QString>()) QString(std::move(*other.cast<QString>()));
        break;
    }
    _type = other._type;
    other._type = NetVariantType::None;
}

void NetVariant::destruct()
{
    switch (_type) {
    case NetVariantType::String:
        cast<std::string>()->~basic_string<char>();
        break;
    case NetVariantType::QString:
        cast<QString>()->~QString();
        break;
    }
}

NetVariant& NetVariant::operator=(const NetVariant& other)
{
    destruct();
    construct(other);
    return *this;
}

NetVariant& NetVariant::operator=(NetVariant&& other)
{
    destruct();
    construct(std::move(other));
    return *this;
}

void NetVariant::serialize(std::string* dest) const
{
    std::uint8_t header = (std::uint8_t)_type;
    bmcl::MemWriter result((void*)dest->data(), dest->size()); // в с++11 строки представлены 1 куском в памяти

    auto reserve = [&result, dest, header](std::size_t size) {
        dest->resize(dest->size() + size + 1);
        result = bmcl::MemWriter((void*)dest->data(), dest->size()); // HACK: после resize может быть realloc
        result.writeUint8(header);
    };

    switch (_type) {
    case NetVariantType::None:
        reserve(0);
        break;
    case NetVariantType::Bool:
        reserve(1);
        result.writeUint8(asBool());
        break;
    case NetVariantType::Int:
        reserve(8);
        result.writeInt64Le(asInt());
        break;
    case NetVariantType::Uint:
        reserve(8);
        result.writeUint64Le(asUint());
        break;
    case NetVariantType::Float:
        reserve(4);
        result.writeFloat32Le(asFloat());
        break;
    case NetVariantType::Double:
        reserve(8);
        result.writeFloat64Le(asDouble());
        break;
    case NetVariantType::String: {
        const std::string& str = asString();
        reserve(8 + str.size() + 1);
        result.writeUint64Le(str.size() + 1);
        result.write(str.data(), str.size());
        break;
    }
    case NetVariantType::QString: {
        const QString& qstr = asQString();
        reserve(8 + qstr.size() * sizeof(QChar));
        result.writeUint64Le(qstr.size());
        result.write(qstr.data(), qstr.size() * sizeof(QChar));
        break;
    }
    }
}

std::string NetVariant::serialize() const
{
    std::string result;
    serialize(&result);
    return result;
}

bmcl::Result<NetVariant, NetVariantError> NetVariant::deserialize(bmcl::MemReader* src)
{
    if (src->readableSize() < 1) {
        return NetVariantError::InvalidSize;
    }
    NetVariantType type = (NetVariantType)src->readUint8();

    std::size_t size;
    if (type == NetVariantType::Float) {
        size = 4;
    } else {
        size = 8;
    }

    if (src->readableSize() < size) {
        return NetVariantError::InvalidSize;
    }

    switch (type) {
    case NetVariantType::None:
        return NetVariant();
    case NetVariantType::Bool:
        return NetVariant(src->readUint8());
    case NetVariantType::Int:
        return NetVariant(src->readInt64Le());
    case NetVariantType::Uint:
        return NetVariant(src->readUint64Le());
    case NetVariantType::Float:
        return NetVariant(src->readFloat32Be());
    case NetVariantType::Double:
        return NetVariant(src->readFloat64Le());
    case NetVariantType::String: {
        std::uint64_t size = src->readUint64Le();
        if (src->readableSize() < size) {
            return NetVariantError::InvalidSize;
        }
        NetVariant var(std::string((const char*)src->current(), size));
        src->skip(size);
        return std::move(var);
    }
    case NetVariantType::QString: {
        std::uint64_t qsize = src->readUint64Le();
        if (src->readableSize() < (qsize * sizeof(QChar))) {
            return NetVariantError::InvalidSize;
        }
        NetVariant qvar(QString((QChar*)src->current(), qsize));
        src->skip(qsize * sizeof(QChar));
        return std::move(qvar);
    }
    }
    return NetVariantError::InvalidHeader;
}

template <typename T, typename S, typename Q>
T NetVariant::to(S stringConverter, Q qstringConverter) const
{
    switch (_type) {
    case NetVariantType::None:
        return 0;
    case NetVariantType::Bool:
        return asBool();
    case NetVariantType::Int:
        return asInt();
    case NetVariantType::Uint:
        return asUint();
    case NetVariantType::Float:
        return asFloat();
    case NetVariantType::Double:
        return asDouble();
    case NetVariantType::String:
        try {
            return stringConverter(asString());
        } catch(const std::exception& e) {
            assert(false);
        }
    case NetVariantType::QString:
        bool isOk;
        T result = qstringConverter(asQString(), &isOk);
        assert(isOk);
        return result;
    }
}

std::int64_t NetVariant::toInt() const
{
    return to<std::int64_t>([](const std::string& str) {
        return std::stoll(str);
    }, [](const QString& str, bool* isOK) {
        return str.toLongLong(isOK);
    });
}

std::uint64_t NetVariant::toUint() const
{
    return to<std::uint64_t>([](const std::string& str) {
        return std::stoull(str);
    }, [](const QString& str, bool* isOK) {
        return str.toULongLong(isOK);
    });
}

float NetVariant::toFloat() const
{
    return to<float>([](const std::string& str) {
        return std::stof(str);
    }, [](const QString& str, bool* isOK) {
        return str.toFloat(isOK);
    });
}

double NetVariant::toDouble() const
{
    return to<double>([](const std::string& str) {
        return std::stod(str);
    }, [](const QString& str, bool* isOK) {
        return str.toDouble(isOK);
    });
}

QVariant NetVariant::toQVariant() const
{
    switch (_type)
    {
    case mcc::misc::NetVariantType::Bool: return QVariant(asBool());
    case mcc::misc::NetVariantType::Int: return QVariant((qlonglong)asInt());
    case mcc::misc::NetVariantType::Uint: return QVariant((qulonglong)asUint());
    case mcc::misc::NetVariantType::Float: return QVariant(asFloat());
    case mcc::misc::NetVariantType::Double: return QVariant(asDouble());
    case mcc::misc::NetVariantType::String: return QVariant(QString::fromStdString(asString()));
    case mcc::misc::NetVariantType::QString: return QVariant(asQString());
    }
    return QVariant();
}

}
}
