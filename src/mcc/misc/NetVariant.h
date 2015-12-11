/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#pragma once

#include "bmcl/MemWriter.h"
#include "bmcl/MemReader.h"
#include "bmcl/Result.h"

#include <QString>
#include <QVariant>

#include <algorithm>
#include <string>
#include <type_traits>
#include <cstdint>
#include <cassert>

namespace mcc {
namespace misc {

template <class... T>
struct AlignedUnion {
private:
#ifdef _MSC_VER
    typename std::aligned_union<0, T...>::type _data;
#else
    template <typename S>
    static constexpr S max(S t)
    {
        return t;
    }

    template <typename S, typename... A>
    static constexpr S max(S t, A... args)
    {
        return t > max(args...) ? t : max(args...);
    }

    alignas(max(alignof(T)...)) char _data[max(sizeof(T)...)];
#endif
};

enum class NetVariantType : std::uint8_t { None, Bool, Int, Uint, Float, Double, String, QString };
enum class NetVariantError { InvalidHeader, InvalidSize };

class NetVariant {
public:
    NetVariant();
    NetVariant(bool value);
    NetVariant(std::int8_t value);
    NetVariant(std::int16_t value);
    NetVariant(std::int32_t value);
    NetVariant(std::int64_t value);
    NetVariant(std::uint8_t value);
    NetVariant(std::uint16_t value);
    NetVariant(std::uint32_t value);
    NetVariant(std::uint64_t value);
    NetVariant(float value);
    NetVariant(double value);
    NetVariant(const std::string& value);
    NetVariant(std::string&& value);
    NetVariant(const QString& value);
    NetVariant(QString&& value);
    NetVariant(const QVariant& value);
    NetVariant(const NetVariant& other);
    NetVariant(NetVariant&& other);
    ~NetVariant();

    // TODO: serialize, deserialize

    NetVariantType type() const;
    bool isBool() const;
    bool isFloat() const;
    bool isDouble() const;
    bool isInt() const;
    bool isUint() const;
    bool isString() const;
    bool isQString() const;

    bool asBool() const;
    std::int64_t asInt() const;
    std::uint64_t asUint() const;
    float asFloat() const;
    double asDouble() const;
    const std::string& asString() const;
    std::string&& asString();
    const QString& asQString() const;
    QString&& asQString();

    std::uint64_t toUint() const;
    std::int64_t toInt() const;
    float toFloat() const;
    double toDouble() const;
    QVariant toQVariant() const;

    std::string stringify() const;
    QString qstringify() const;

    void serialize(std::string* dest) const;
    std::string serialize() const;
    static bmcl::Result<NetVariant, NetVariantError> deserialize(bmcl::MemReader* src);

    NetVariant& operator=(const NetVariant& other);
    NetVariant& operator=(NetVariant&& other);

private:
    void destruct();
    void construct(const NetVariant& other);
    void construct(NetVariant&& other);

    template <typename T>
    T* cast();

    template <typename T, typename S, typename Q>
    T to(S stringConverter, Q qstringConverter) const;

    template <typename T>
    const T* cast() const;

    template <typename T>
    void init(T&& value, NetVariantType type);

    AlignedUnion<std::int64_t, std::uint64_t, float, double, std::string, QString> _data;
    NetVariantType _type;
};

template <typename T>
inline void NetVariant::init(T&& value, NetVariantType type)
{
    typedef typename std::decay<T>::type U;
    new (cast<U>()) U(std::forward<T>(value));
    _type = type;
}

inline NetVariant::NetVariant()
    : _type(NetVariantType::None)
{
}

inline NetVariant::NetVariant(bool value)
{
    init(value, NetVariantType::Bool);
}

inline NetVariant::NetVariant(std::int8_t value)
{
    init(std::int64_t(value), NetVariantType::Int);
}

inline NetVariant::NetVariant(std::int16_t value)
{
    init(std::int64_t(value), NetVariantType::Int);
}

inline NetVariant::NetVariant(std::int32_t value)
{
    init(std::int64_t(value), NetVariantType::Int);
}

inline NetVariant::NetVariant(std::int64_t value)
{
    init(value, NetVariantType::Int);
}

inline NetVariant::NetVariant(std::uint8_t value)
{
    init(std::uint64_t(value), NetVariantType::Uint);
}

inline NetVariant::NetVariant(std::uint16_t value)
{
    init(std::uint64_t(value), NetVariantType::Uint);
}

inline NetVariant::NetVariant(std::uint32_t value)
{
    init(std::uint64_t(value), NetVariantType::Uint);
}

inline NetVariant::NetVariant(std::uint64_t value)
{
    init(value, NetVariantType::Uint);
}

inline NetVariant::NetVariant(float value)
{
    init(value, NetVariantType::Float);
}

inline NetVariant::NetVariant(double value)
{
    init(value, NetVariantType::Double);
}

inline NetVariant::NetVariant(const std::string& value)
{
    init(value, NetVariantType::String);
}

inline NetVariant::NetVariant(std::string&& value)
{
    init(std::move(value), NetVariantType::String);
}

inline NetVariant::NetVariant(const QString& value)
{
    init(value, NetVariantType::QString);
}

inline NetVariant::NetVariant(QString&& value)
{
    init(std::move(value), NetVariantType::QString);
}

inline NetVariant::NetVariant(const NetVariant& other)
{
    construct(other);
}

inline NetVariant::NetVariant(NetVariant&& other)
{
    construct(std::move(other));
}

inline NetVariant::~NetVariant()
{
    destruct();
}

inline NetVariantType NetVariant::type() const
{
    return _type;
}

inline bool NetVariant::isBool() const
{
    return _type == NetVariantType::Bool;
}

inline bool NetVariant::isInt() const
{
    return _type == NetVariantType::Int;
}

inline bool NetVariant::isUint() const
{
    return _type == NetVariantType::Uint;
}

inline bool NetVariant::isFloat() const
{
    return _type == NetVariantType::Float;
}

inline bool NetVariant::isDouble() const
{
    return _type == NetVariantType::Double;
}

inline bool NetVariant::isString() const
{
    return _type == NetVariantType::String;
}

inline bool NetVariant::isQString() const
{
    return _type == NetVariantType::QString;
}

template <typename T>
inline T* NetVariant::cast()
{
    return reinterpret_cast<T*>(&_data);
}

template <typename T>
inline const T* NetVariant::cast() const
{
    return reinterpret_cast<const T*>(&_data);
}

inline bool NetVariant::asBool() const
{
    assert(isBool());
    return *cast<bool>();
}

inline std::int64_t NetVariant::asInt() const
{
    assert(isInt());
    return *cast<std::int64_t>();
}

inline std::uint64_t NetVariant::asUint() const
{
    assert(isUint());
    return *cast<std::uint64_t>();
}

inline float NetVariant::asFloat() const
{
    assert(isFloat());
    return *cast<float>();
}

inline double NetVariant::asDouble() const
{
    assert(isDouble());
    return *cast<double>();
}

inline const std::string& NetVariant::asString() const
{
    assert(isString());
    return *cast<std::string>();
}

inline std::string&& NetVariant::asString()
{
    assert(isString());
    return std::move(*cast<std::string>());
}

inline const QString& NetVariant::asQString() const
{
    assert(isQString());
    return *cast<QString>();
}

inline QString&& NetVariant::asQString()
{
    assert(isQString());
    return std::move(*cast<QString>());
}
}
}
