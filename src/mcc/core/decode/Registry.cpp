/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include <utility>
#include <limits>

#include <QString>
#include <QStringList>
#include <QDebug>

#include "bmcl/Logging.h"
#include "bmcl/Assert.h"
#include "bmcl/Panic.h"

#include "mcc/misc/cast_utils.h"

#include "mcc/core/decode/Registry.h"

#define IFDEV_CHECK_VALUE_OR_RETURN(v) if (v.isError()) { return ValueOrError(std::move(v)); }
#define IFDEV_CHECK_MEMWRITER_SIZE_TYPE(writer, t) if (writer.sizeLeft() < sizeof(t)) { return Option<std::string>("not enough space to write"); }

using mcc::utils::checked_cast;

namespace mcc
{
namespace decode
{

PrimitiveTypeKind primitiveTypeKindOf(const std::string &value)
{
    if (value == "float")
    {
        return PrimitiveTypeKind::Float;
    }
    else if (value == "uint")
    {
        return PrimitiveTypeKind::Uint;
    }
    else if (value == "int")
    {
        return PrimitiveTypeKind::Int;
    }
    else if (value == "bool")
    {
        return PrimitiveTypeKind::Bool;
    }
    bmcl::panic("invalid type kind value");
}

ValueOrError PrimitiveType::deserialize(MemReader &reader) const
{
    Variant *result = nullptr;
    switch (_typeKind)
    {
    case PrimitiveTypeKind::Float:
        switch (_bitLength)
        {
        case 32:
            result = new PrimitiveVariantTpl<float>(reader.readFloat32Le());
            break;
        case 64:
            result = new PrimitiveVariantTpl<double>(reader.readFloat64Le());
            break;
        default:
            bmcl::panic("float bit size must be 32 or 64");
        }
        break;
    case PrimitiveTypeKind::Int:
        switch (_bitLength)
        {
        case 8:
            result = new PrimitiveVariantTpl<int8_t>(reader.readInt8());
            break;
        case 16:
            result = new PrimitiveVariantTpl<int16_t>(reader.readInt16Le());
            break;
        case 32:
            result = new PrimitiveVariantTpl<int32_t>(reader.readInt32Le());
            break;
        case 64:
            result = new PrimitiveVariantTpl<int64_t>(reader.readInt64Le());
            break;
        default:
            bmcl::panic("invalid int size");
        }
        break;
    case PrimitiveTypeKind::Uint:
        switch (_bitLength)
        {
        case 8:
            result = new PrimitiveVariantTpl<uint8_t>(reader.readUint8());
            break;
        case 16:
            result = new PrimitiveVariantTpl<uint16_t>(reader.readUint16Le());
            break;
        case 32:
            result = new PrimitiveVariantTpl<uint32_t>(reader.readUint32Le());
            break;
        case 64:
            result = new PrimitiveVariantTpl<uint64_t>(reader.readUint64Le());
            break;
        default:
            bmcl::panic("invalid uint size");
        }
        break;
    case PrimitiveTypeKind::Bool:
        switch (_bitLength)
        {
        case 8:
            result = new PrimitiveVariantTpl<bool>(reader.readUint8() != 0);
            break;
        case 16:
            result = new PrimitiveVariantTpl<bool>(reader.readUint16Le() != 0);
            break;
        case 32:
            result = new PrimitiveVariantTpl<bool>(reader.readUint32Le() != 0);
            break;
        case 64:
            result = new PrimitiveVariantTpl<bool>(reader.readUint64Le() != 0);
            break;
        default:
            bmcl::panic("invalid int size");
        }
        break;
    default:
        bmcl::panic("invalid primitive type kind");
    }
    return ValueOrError(result);
}

Option<std::string> PrimitiveType::serialize(const Variant &value, MemWriter &writer) const
{
    // Variant *result = nullptr;
    switch (_typeKind)
    {
    case PrimitiveTypeKind::Float:
        switch (_bitLength)
        {
        case 32:
            IFDEV_CHECK_MEMWRITER_SIZE_TYPE(writer, float);
            writer.writeFloat32Le(value.toFloat32());
            break;
        case 64:
            IFDEV_CHECK_MEMWRITER_SIZE_TYPE(writer, double);
            writer.writeFloat64Le(value.toFloat64());
            break;
        default:
            bmcl::panic("float bit size must be 32 or 64");
        }
        break;
    case PrimitiveTypeKind::Int:
        switch (_bitLength)
        {
        case 8:
            IFDEV_CHECK_MEMWRITER_SIZE_TYPE(writer, int8_t);
            writer.writeInt8(value.toInt8());
            break;
        case 16:
            IFDEV_CHECK_MEMWRITER_SIZE_TYPE(writer, int16_t);
            writer.writeInt16Le(value.toInt16());
            break;
        case 32:
            IFDEV_CHECK_MEMWRITER_SIZE_TYPE(writer, int32_t);
            writer.writeInt32Le(value.toInt32());
            break;
        case 64:
            IFDEV_CHECK_MEMWRITER_SIZE_TYPE(writer, int64_t);
            writer.writeInt64Le(value.toInt64());
            break;
        default:
            bmcl::panic("invalid int size");
        }
        break;
    case PrimitiveTypeKind::Uint:
        switch (_bitLength)
        {
        case 8:
            IFDEV_CHECK_MEMWRITER_SIZE_TYPE(writer, uint8_t);
            writer.writeUint8(value.toUint8());
            break;
        case 16:
            IFDEV_CHECK_MEMWRITER_SIZE_TYPE(writer, uint16_t);
            writer.writeUint16Le(value.toUint16());
            break;
        case 32:
            IFDEV_CHECK_MEMWRITER_SIZE_TYPE(writer, uint32_t);
            writer.writeUint32Le(value.toUint32());
            break;
        case 64:
            IFDEV_CHECK_MEMWRITER_SIZE_TYPE(writer, uint64_t);
            writer.writeUint64Le(value.toUint64());
            break;
        default:
            bmcl::panic("invalid uint size");
        }
        break;
    case PrimitiveTypeKind::Bool:
        switch (_bitLength)
        {
        case 8:
            IFDEV_CHECK_MEMWRITER_SIZE_TYPE(writer, uint8_t);
            writer.writeUint8(value.toBool());
            break;
        case 16:
            IFDEV_CHECK_MEMWRITER_SIZE_TYPE(writer, uint16_t);
            writer.writeUint16Le(value.toBool());
            break;
        case 32:
            IFDEV_CHECK_MEMWRITER_SIZE_TYPE(writer, uint32_t);
            writer.writeUint32Le(value.toBool());
            break;
        case 64:
            IFDEV_CHECK_MEMWRITER_SIZE_TYPE(writer, uint64_t);
            writer.writeUint64Le(value.toBool());
            break;
        default:
            bmcl::panic("invalid int size");
        }
        break;
    default:
        bmcl::panic("invalid primitive type kind");
    }
    return Option<std::string>();
}

EnumType::EnumType(const Option<Name> &name, const std::weak_ptr<Namespace> & _namespace, const Option<std::string> &info,
                   const std::weak_ptr<Type> & baseType,
                   std::vector<std::shared_ptr<EnumConstant>> && constants)
        : Type(name, _namespace, info), HasBaseType(baseType), _constants(std::move(constants))
{
    for (auto& constant : _constants)
    {
        _constantsByName.emplace(constant->name(), constant);
    }
}

ValueOrError StructType::deserialize(MemReader &reader) const
{
    MapVariant *map(new MapVariant());
    for (auto & field : _fields)
    {
        ValueOrError readingResult = field->baseType()->deserialize(reader);
        IFDEV_CHECK_VALUE_OR_RETURN(readingResult);
        map->emplace(field->name(), VariantPtr(readingResult.value().release()));
    }
    return ValueOrError(map);
}

Option<std::string> StructType::serialize(const Variant &value, MemWriter &writer) const
{
    const MapVariant &map(value.asMap());
    for (auto &field : _fields)
    {
        Option<std::string> result = field->baseType()->serialize(map.at(field->name()), writer);
        if (result.isSome())
        {
            return result;
        }
    }
    return Option<std::string>();
}

void StructType::updateIndices()
{
    _fieldsByName.clear();
    BMCL_ASSERT_MSG(_fields.size() != 0, "struct must not be empty");
    for(auto& field : _fields)
    {
        _fieldsByName.emplace(field->name(), field);
    }
}

ArrayType::ArrayType(const Option<Name> &name, const std::weak_ptr<Namespace> & _namespace,
                     const Option<std::string> &info,
                     const std::weak_ptr<Type> & baseType, uint_fast64_t minLength,
                     uint_fast64_t maxLength)
        : Type(name, _namespace, info), HasBaseType(baseType), _minLength(minLength), _maxLength(maxLength)
{
    BMCL_ASSERT_MSG(_maxLength > 0, "maxLength must be positive");
    BMCL_ASSERT_MSG(_minLength <= _maxLength, "minLength must not be greater than maxLength");
}

ValueOrError ArrayType::deserialize(MemReader &reader) const
{
    MapVariant *map(new MapVariant());
    if (_minLength == _maxLength)
    {
        for (std::size_t index(0); index < _maxLength; ++index)
        {
            ValueOrError readingResult = baseType()->deserialize(reader);
            IFDEV_CHECK_VALUE_OR_RETURN(readingResult);
            map->emplace(std::to_string(index), VariantPtr(readingResult.value().release()));
        }
    }
    else
    {
        uint_fast32_t size(_maxLength > std::numeric_limits<uint16_t>::max()
            ? reader.readUint32Le()
            : (_maxLength > std::numeric_limits<uint8_t>::max()
                ? reader.readUint16Le()
                : reader.readUint8()));
        BMCL_ASSERT_MSG(size >= _minLength && size <= _maxLength, QString("dynamic array size out of bounds (%1 <= %2 <= %3)").arg(_minLength).arg(size).arg(_maxLength).toStdString().c_str());
        map->emplace("size", VariantPtr(new PrimitiveVariantTpl<uint_fast32_t>(size)));
        for (std::size_t index(0); index < size; ++index)
        {
            ValueOrError readingResult(baseType()->deserialize(reader));
            IFDEV_CHECK_VALUE_OR_RETURN(readingResult);
            map->emplace(std::to_string(index), VariantPtr(readingResult.value().release()));
        }
    }
    return ValueOrError(map);
}

Option<std::string> ArrayType::serialize(const Variant &value, MemWriter &writer) const
{
    const MapVariant &map(value.asMap());
    uint_fast64_t size = _minLength;
    if (!isFixedSize())
    {
        size = map.at("size").toUint64();
        if (size < _minLength || size > _maxLength)
        {
            return Option<std::string>("array size is out of bounds");
        }
        if (_maxLength > std::numeric_limits<uint8_t>::max())
        {
            if (_maxLength > std::numeric_limits<uint16_t>::max())
            {
                if (_maxLength > std::numeric_limits<uint32_t>::max())
                {
                    IFDEV_CHECK_MEMWRITER_SIZE_TYPE(writer, uint64_t);
                    writer.writeUint64Le(size);
                }
                else
                {
                    IFDEV_CHECK_MEMWRITER_SIZE_TYPE(writer, uint32_t);
                    writer.writeUint32Le(checked_cast<uint32_t>(size));
                }
            }
            else
            {
                IFDEV_CHECK_MEMWRITER_SIZE_TYPE(writer, uint16_t);
                writer.writeUint16Le(checked_cast<uint16_t>(size));
            }
        }
        else
        {
            IFDEV_CHECK_MEMWRITER_SIZE_TYPE(writer, uint8_t);
            writer.writeUint8(checked_cast<uint8_t>(size));
        }
    }
    for (std::size_t index(0); index < size; index++)
    {
        Option<std::string> result(baseType()->serialize(map.at(std::to_string(index)), writer));
        if (result.isSome())
        {
            return result;
        }
    }
    return Option<std::string>();
}

Registry::Registry(std::vector<std::shared_ptr<Namespace>> && rootNamespaces)
        : _rootNamespaces(rootNamespaces)
{
    for(auto & ns : _rootNamespaces)
    {
        _rootNamespacesByName.emplace(ns->name(), ns);
    }
}

std::shared_ptr<Message> Registry::messageByFqn(const std::string& fqn)
{
    std::size_t messageNamePos(fqn.find_last_of('.'));
    BMCL_ASSERT_MSG(messageNamePos != std::string::npos, QString("invalid FQN for message '%1'").arg(messageNamePos).toStdString().c_str());
    auto component = componentByFqn(fqn.substr(0, messageNamePos));
    if(component != nullptr)
    {
        return component->messageByName(fqn.substr(messageNamePos + 1));
    }
    return std::shared_ptr<Message>();
}

std::shared_ptr<Component> Registry::componentByFqn(const std::string &fqn)
{
    std::size_t componentNamePos = fqn.find_last_of('.');
    BMCL_ASSERT_MSG(componentNamePos != std::string::npos, QString("invalid FQN for component '%1'").arg(componentNamePos).toStdString().c_str());
    if(std::shared_ptr<Namespace> _namespace = namespaceByFqn(fqn.substr(0, componentNamePos)))
        return _namespace->componentByName(fqn.substr(componentNamePos + 1));
    return std::shared_ptr<Component>();
}

std::shared_ptr<Namespace> Registry::namespaceByFqn(const std::string &fqn)
{
    std::size_t startPos = 0;
    std::shared_ptr<Namespace> _namespace;
    while (std::size_t dotPos = fqn.find_first_of('.', startPos))
    {
        if (dotPos == std::string::npos)
            break;
        _namespace = _namespace
                     ? _namespace->subNamespaceByName(fqn.substr(startPos, dotPos - startPos))
                     : rootNamespaceByName(fqn.substr(startPos, dotPos - startPos));

        if (!_namespace)
            return _namespace;
        startPos = dotPos + 1;
    }
    _namespace = _namespace
                 ? _namespace->subNamespaceByName(fqn.substr(startPos))
                 : rootNamespaceByName(fqn.substr(startPos));
    return _namespace;
}

std::shared_ptr<Type> Message::findTypeForParameter(const MessageParameter &parameter) const
{
    std::shared_ptr<Component> currentComponent = component();
    std::shared_ptr<Type> type;
    bool isFirstPart = true;
    for (auto & paramPart : QString::fromStdString(parameter.value()).split('.'))
    {
        if (!type) // В начале параметра могут быть использованы подкомпоненты
        {
            if (std::shared_ptr<Component> subComponent = currentComponent->subComponentByName(paramPart.toStdString()))
            {
                currentComponent = subComponent;
                continue;
            }
        }
        if (isFirstPart && !type)
            type = currentComponent->baseType();
        isFirstPart = false;
        StructType *structType = dynamic_cast<StructType*>(type.get());
        bool isArrayElement(false);
        if (paramPart.contains("["))
        {
            isArrayElement = true;
            paramPart = paramPart.mid(0, paramPart.indexOf("["));
        }
        BMCL_ASSERT_MSG(structType != nullptr, QString("expected struct when processing '%1' part of parameter '%2'").arg(paramPart).arg(QString::fromStdString(parameter.value())).toStdString().c_str());
        std::shared_ptr<StructField> field = structType->fieldByName(paramPart.toStdString());
        BMCL_ASSERT_MSG(field, QString("field '%1' not found for struct '%2'").arg(paramPart).arg(QString::fromStdString(structType->nameOptional().unwrapOr("noname"))).toStdString().c_str());
        type = field->baseType();
        if (isArrayElement)
        {
            AsArrayTypeVisitor asArrayTypeVisitor;
            type->accept(asArrayTypeVisitor);
            BMCL_ASSERT_MSG(asArrayTypeVisitor.hasResult(), "expected array");
            ArrayType & arrayType = asArrayTypeVisitor.result();
            type = arrayType.baseType();
        }
    }
    BMCL_ASSERT_MSG(type, "type not found");
    return type;
}

QDebug operator<<(QDebug dbg, const ValueOrError &v)
{
    if (v.isError())
    {
        dbg << "Error";
    }
    else
    {
        dbg << *v.value().get();
    }
    return dbg;
}

ValueOrError Message::deserialize(MemReader &reader) const
{
    Variant *result = nullptr;
    bool starParameterFound = false;
    for(auto const& parameter : parameters())
    {
        const std::string &value(parameter->value());
        if (value == "*.*")
        {
            BMCL_ASSERT_MSG(result == nullptr, "*.* must be the only one parameter in message");
            MapVariant *map(new MapVariant());
            result = map;
            for (auto& subComponent : component()->uniqueOrderedSubComponentSet())
            {
                if (std::shared_ptr<Type> baseType = subComponent->baseType())
                {
                    ValueOrError readingResult = baseType->deserialize(reader);
                    IFDEV_CHECK_VALUE_OR_RETURN(readingResult);
                    map->emplace(subComponent->name(), VariantPtr(readingResult.value().release()));
                }
            }
            if (std::shared_ptr<Type> type = component()->baseType())
            {
                ValueOrError readingResult(type->deserialize(reader));
                IFDEV_CHECK_VALUE_OR_RETURN(readingResult);
                Variant *variant(readingResult.value().get());
                MapVariant *mapVariant(dynamic_cast<MapVariant*>(variant));
                if (mapVariant != nullptr)
                {
                    auto &map2(mapVariant->map());
                    for (auto& entry : map2)
                    {
                        map->emplace(entry.first, entry.second);
                    }
                    map2.clear();
                }
                else
                {
                    //parametersMap.emplace("*", std::move(variant));
                }
            }
            break;
        }
        if (value == "*")
        {
            BMCL_ASSERT_MSG(!starParameterFound, "multiple '*' parameters are not allowed in message");
            std::shared_ptr<Type> baseType = component()->baseType();
            BMCL_ASSERT_MSG(baseType, "'*' parameter with empty component base type");
            ValueOrError readingResult(baseType->deserialize(reader));
            IFDEV_CHECK_VALUE_OR_RETURN(readingResult);
            if (result == nullptr)
            {
                result = readingResult.value().get();
            }
            else
            {
                MapVariant *resultMap(dynamic_cast<MapVariant*>(result));
                BMCL_ASSERT_MSG(resultMap != nullptr, "can't merge into PrimitiveVariant");
                Variant *baseTypeValue(readingResult.value().get());
                MapVariant *baseTypeMap(dynamic_cast<MapVariant*>(baseTypeValue));
                if (baseTypeMap != nullptr)
                {
                    auto &map(baseTypeMap->map());
                    for (auto& entry : map)
                    {
                        resultMap->emplace(entry.first, entry.second);
                    }
                    map.clear();
                }
                else
                {
                    resultMap->emplace("*", VariantPtr(readingResult.value().release()));
                }
            }
            starParameterFound = true;
        }

        std::shared_ptr<Type> parameterType = findTypeForParameter(*parameter);
        ValueOrError readingResult = parameterType->deserialize(reader);
        IFDEV_CHECK_VALUE_OR_RETURN(readingResult);
        if (result == nullptr)
        {
            result = new MapVariant();
        }
        result->setValue(value, VariantPtr(readingResult.value().release()));
    }
    BMCL_ASSERT_MSG(result != nullptr, "message read failed");
    return ValueOrError(result);
}

Option<std::string> Message::serialize(MapVariant& map, MemWriter& writer) const
{
    bmcl::panic("not implemented");
}

Option<std::string> Command::serialize(MapVariant& map, MemWriter& writer) const
{
    for (auto& arg : _arguments)
    {
        Variant &argValue = map.at(arg->name());
        Option<std::string> err = arg->baseType()->serialize(argValue, writer);
        if (err.isSome())
        {
            return err;
        }
    }
    return Option<std::string>();
}


std::string Component::fqn() const
{
    return _namespace()->fqn() + std::string(".") + _name;
}

UniqueOrderedComponentSet Component::uniqueOrderedSubComponentSet()
{
    UniqueOrderedComponentSet result;
    appendSubComponentsToSet(result);
    BMCL_ASSERT_MSG(result.find(this) == result.end(), "sub components ordered set must not include component itself");
    return result;
}

void Component::appendSubComponentsToSet(UniqueOrderedComponentSet &set)
{
    for (auto& subComponent : _subComponents)
    {
        std::shared_ptr<Component> s = subComponent.lock();
        set.insert(s.get());
        s->appendSubComponentsToSet(set);
    }
}


void Component::updateIndices()
{
    _subComponentsByName.clear();
    for(auto & subComponent : _subComponents)
    {
        std::shared_ptr<Component> s = subComponent.lock();
        _subComponentsByName.emplace(s->name(), s);
    }
    _commandsByName.clear();
    _commandsById.clear();
    for(auto & command : _commands)
    {
        _commandsByName.emplace(command->name(), command);
        _commandsById.emplace(command->id(), command);
    }
    _messagesByName.clear();
    _messagesById.clear();
    for(auto& message : _messages)
    {
        _messagesByName.emplace(message->name(), message);
        _messagesById.emplace(message->id(), message);
    }
}

void Namespace::updateIndices()
{
    _subNamespaceByName.clear();
    for(auto& subNamespace : _subNamespaces)
    {
        _subNamespaceByName.emplace(subNamespace->name(), subNamespace);
    }
    _unitByName.clear();
    for(auto& unit : _units)
    {
        _unitByName.emplace(unit->name(), unit);
    }
    _typeByName.clear();
    for(auto & type : _types)
    {
        type->updateIndices();
        Option<Name> nameOptional(type->nameOptional());
        if (nameOptional.isSome())
        {
            _typeByName.emplace(nameOptional.unwrap(), type);
        }
    }
    _componentByName.clear();
    for(auto& component : _components)
    {
        component->updateIndices();
        _componentByName.emplace(component->name(), component);
    }
}

}
}
