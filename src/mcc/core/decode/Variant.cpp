/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include <algorithm>
#include <string>

#include "bmcl/Logging.h"

#include <QString>

#include "mcc/core/decode/Variant.h"

namespace mcc
{
namespace decode
{

QDebug operator<<(QDebug dbg, const Variant &v)
{
    return dbg << QString::fromStdString(v.to_string().c_str());
}

MapVariant &Variant::asMap() { return *static_cast<MapVariant*>(this); }
const MapVariant &Variant::asMap() const { return *static_cast<const MapVariant*>(this); }

PrimitiveVariant &Variant::asPrimitive() { return *static_cast<PrimitiveVariant*>(this); }
const PrimitiveVariant &Variant::asPrimitive() const { return *static_cast<const PrimitiveVariant*>(this); }

const Variant &MapVariant::getValue(const std::string &name) const
{
    const MapVariant *currentMap = this;
    std::size_t currentPos(0);
    std::size_t delimiterPos;
    std::string processedName(processName(name));
    while ((delimiterPos = processedName.find_first_of(".[]", currentPos)) != std::string::npos)
    {
        if (delimiterPos != currentPos)
        {
            std::string part(processedName.substr(currentPos, delimiterPos - currentPos));
            auto it(currentMap->map().find(part));
            BMCL_ASSERT_MSG(it != currentMap->map().end(), QString("'%1' not found in '%2'").arg(QString::fromStdString(part)).arg(QString::fromStdString(currentMap->to_string())).toStdString().c_str());
            currentMap = dynamic_cast<MapVariant*>(it->second.get());
            BMCL_ASSERT_MSG(currentMap != nullptr, QString("'%1' must ba a map").arg(QString::fromStdString(it->second->to_string())).toStdString().c_str());
        }
        currentPos = delimiterPos + 1;
    }
    return currentMap->at(processedName.substr(currentPos));
}

uint64_t Variant::toUint64() const
{
    return asPrimitive().toUint64();
}

uint32_t Variant::toUint32() const
{
    return asPrimitive().toUint32();
}

uint16_t Variant::toUint16() const
{
    return asPrimitive().toUint16();
}

uint8_t Variant::toUint8() const
{
    return asPrimitive().toUint8();
}

int64_t Variant::toInt64() const
{
    return asPrimitive().toInt64();
}

int32_t Variant::toInt32() const
{
    return asPrimitive().toInt32();
}

int16_t Variant::toInt16() const
{
    return asPrimitive().toInt16();
}

int8_t Variant::toInt8() const
{
    return asPrimitive().toInt8();
}

float Variant::toFloat32() const
{
    return asPrimitive().toFloat32();
}

double Variant::toFloat64() const
{
    return asPrimitive().toFloat64();
}

bool Variant::toBool() const
{
    return asPrimitive().toBool();
}

void Variant::setValue(const std::string &name, VariantPtr &&value)
{
    asMap().setValue(name, std::move(value));
}

void MapVariant::setValue(const std::string &name, VariantPtr &&value)
{
    MapVariant *currentMap = this;
    std::size_t currentPos(0);
    std::size_t delimiterPos;
    std::string processedName(processName(name));
    while ((delimiterPos = processedName.find_first_of(".[]", currentPos)) != std::string::npos)
    {
        if (delimiterPos != currentPos)
        {
            std::string part(processedName.substr(currentPos, delimiterPos - currentPos));
            auto it(currentMap->map().find(part));
            if (it == currentMap->map().end())
            {
                MapVariant *newMap = new MapVariant();
                currentMap->emplace(part, VariantPtr(newMap));
                currentMap = newMap;
            }
            else
            {
                currentMap = dynamic_cast<MapVariant*>(it->second.get());
                BMCL_ASSERT_MSG(currentMap != nullptr, "must ba a map");
            }
        }
        currentPos = delimiterPos + 1;
    }
    std::string lastPart(processedName.substr(currentPos));
    BMCL_ASSERT_MSG(!lastPart.empty(), QString("insertion error for '%1'").arg(QString::fromStdString(name)).toStdString().c_str());
    currentMap->emplace(lastPart, std::move(value));
}

std::string MapVariant::to_string() const
{
    std::ostringstream str("MapVariant", std::ostringstream::ate);
    str << "(" << std::to_string(_map.size()) << " elements){";
    for (auto& entry : _map)
    {
        str << entry.first << "=" << entry.second->to_string() << ",";
    }
    str << "}";
    return str.str();
}

}
}