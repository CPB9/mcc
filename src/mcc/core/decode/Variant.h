/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#pragma once

#include <cstdint>
#include <map>
#include <cmath>
#include <utility>
#include <memory>
#include <sstream>

#include <QString>
#include <QDebug>

#include "bmcl/Panic.h"
#include "bmcl/Assert.h"
#include "bmcl/Option.h"

#include "mcc/misc/cast_utils.h"

#include "mcc/core/decode/Common.h"

namespace mcc
{
namespace decode
{

class Variant;
class MapVariant;
class PrimitiveVariant;
template<typename T>
class PrimitiveVariantTpl;

typedef std::unique_ptr<Variant> VariantPtr;

class Variant
{
public:
    virtual ~Variant() {}
    virtual bool isPrimitive() const { return false; }
    virtual bool isMap() const { return false; }
    virtual std::string to_string() const = 0;

    template<typename T>
    VariantPtr fromValue(T&& value);


    MapVariant & asMap();
    const MapVariant & asMap() const;
    PrimitiveVariant & asPrimitive();
    const PrimitiveVariant & asPrimitive() const;

    uint64_t toUint64() const;
    uint32_t toUint32() const;
    uint16_t toUint16() const;
    uint8_t toUint8() const;
    int64_t toInt64() const;
    int32_t toInt32() const;
    int16_t toInt16() const;
    int8_t toInt8() const;
    float toFloat32() const;
    double toFloat64() const;
    bool toBool() const;
    void setValue(const std::string &name, VariantPtr &&value);
};

QDebug operator<<(QDebug dbg, const Variant &v);

class PrimitiveVariant : public Variant
{
public:
    virtual bool isPrimitive() const override { return true; }
    virtual uint64_t toUint64() const = 0;
    virtual uint32_t toUint32() const = 0;
    virtual uint16_t toUint16() const = 0;
    virtual uint8_t toUint8() const = 0;
    virtual int64_t toInt64() const = 0;
    virtual int32_t toInt32() const = 0;
    virtual int16_t toInt16() const = 0;
    virtual int8_t toInt8() const = 0;
    virtual float toFloat32() const = 0;
    virtual double toFloat64() const = 0;
    virtual bool toBool() const = 0;
};

template<typename T>
class PrimitiveVariantTpl : public PrimitiveVariant
{
public:
    explicit PrimitiveVariantTpl(T value) : _value(value) {}

    T value() const { return _value; }
    // TODO FIXME: заменить шаблоном и исправить знаковое-беззнаковое сравнение
    uint64_t toUint64() const override { return utils::checked_cast<uint64_t>(_value); }
    uint32_t toUint32() const override { return utils::checked_cast<uint32_t>(_value); }
    uint16_t toUint16() const override { return utils::checked_cast<uint16_t>(_value); }
    uint8_t toUint8() const override { return utils::checked_cast<uint8_t>(_value); }
    int64_t toInt64() const override { return utils::checked_cast<int64_t>(_value); }
    int32_t toInt32() const override { return utils::checked_cast<int32_t>(_value); }
    int16_t toInt16() const override { return utils::checked_cast<int16_t>(_value); }
    int8_t toInt8() const override { return utils::checked_cast<int8_t>(_value); }
    float toFloat32() const override { return _value; }
    double toFloat64() const override { return _value; }
    bool toBool() const override { return _value; }
    std::string to_string() const override { std::ostringstream str("PrimitiveVariant{", std::ostringstream::ate); str << std::to_string(_value) << "}"; return str.str(); }
private:
    T _value;
};

template<typename T>
inline VariantPtr Variant::fromValue(T&& value)
{
    return VariantPtr(new PrimitiveVariantTpl<T>(value));
}

class MapVariant : public Variant
{
public:
    explicit MapVariant() {}
    MapVariant(MapVariant& map) = delete;
    MapVariant(MapVariant&& map) : _map(std::move(map._map)) {}
    virtual bool isMap() const override { return true; }

    void emplace(const std::string &key, VariantPtr&& value) { BMCL_ASSERT_MSG(value != nullptr, "invalid value (nullptr)"); _map.emplace(key, std::move(value)); }
    void emplace(const std::string &key, VariantPtr& value) { emplace(key, std::move(value)); }

    template<typename T>
    MapVariant &with(const std::string &key, T && value) { _map.emplace(key, std::move(Variant::fromValue(std::forward<T>(value)))); return *this; }

    std::map<std::string, VariantPtr> &map() { return _map; }
    const std::map<std::string, VariantPtr> &map() const { return _map; }

    std::size_t size() const { return _map.size(); }
    bool empty() const { return _map.empty(); }
    Variant &at(const std::string &key)
    {
        BMCL_ASSERT_MSG(_map.find(key) != _map.end(), QString("no key '%1' in '%2'").arg(QString::fromStdString(key)).arg(QString::fromStdString(to_string())).toStdString().c_str());
        return *_map.at(key);
    }
    const Variant &at(const std::string &key) const
    {
        BMCL_ASSERT_MSG(_map.find(key) != _map.end(), QString("no key '%1' in '%2'").arg(QString::fromStdString(key)).arg(QString::fromStdString(to_string())).toStdString().c_str());
        return *_map.at(key);
    }

    const Variant &getValue(const std::string &name) const;
    void setValue(const std::string &name, VariantPtr &&value);

    std::string to_string() const override;

private:
    static std::string processName(std::string name) { name.erase(std::remove_if(name.begin(), name.end(), [](const char ch){ return ch == ']'; }), name.end()); return name; }

private:
    std::map<std::string, VariantPtr> _map;

};

template<> inline VariantPtr Variant::fromValue<MapVariant&>(MapVariant& value) { return VariantPtr(new MapVariant(std::move(value))); }
template<> inline VariantPtr Variant::fromValue<MapVariant&&>(MapVariant&& value) { return VariantPtr(new MapVariant(std::move(value))); }

}
}
