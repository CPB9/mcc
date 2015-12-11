/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#pragma once

#include <cstdint>

#include <map>
#include <set>
#include <vector>
#include <string>
#include <memory>
#include <utility>

#include <QDebug>

#include "bmcl/Assert.h"
#include "bmcl/Option.h"
#include "bmcl/Panic.h"
#include "bmcl/MemReader.h"
#include "bmcl/MemWriter.h"
#include "bmcl/Either.h"

#include "mcc/core/decode/Common.h"
#include "mcc/core/decode/Variant.h"

#define IFDEV_MAKE_ERROR(msg) std::unique_ptr<Error>(new Error("File "##__FILE__##":"##__LINE__##": "##msg)))
#define IFDEV_MAKE_ERROR_WITH_CAUSE(msg, cause) std::unique_ptr<Error>(new Error("File "##__FILE__##":"##__LINE__##": "##msg, cause)))

namespace mcc
{
namespace decode
{

using bmcl::Option;
using bmcl::MemReader;
using bmcl::MemWriter;
using bmcl::Either;

//! Тип имени может быть изменен в будущем, потому что не любая строка это валидное имя
using Name = std::string;

class Error
{
public:
    explicit Error(const std::string & msg) : _msg(msg) {}
    const std::string & msg() const { return _msg; }
private:
    std::string _msg;
};

/*!
 * Базовый класс для всех элементов у которых может быть имя
 */
class HasNameOptional
{
public:
    virtual Option<Name> nameOptional() = 0;
protected:
    // Запрещаем удаление через укзатель на этот класс
    virtual ~HasNameOptional() {}
};

/*!
 * Базовый класс всех элементов у которых точно есть имя
 */
class HasName : public HasNameOptional
{
public:
    virtual const Name& name() const = 0;
protected:
    Option<Name> nameOptional() override { return Option<Name>(name()); }
    // Запрещаем удаление через укзатель на этот класс
    virtual ~HasName() {}
};

class HasInfoOptional
{
public:
    Option<std::string> info() { return _info; }
    const Option<std::string> & info() const { return _info; }
protected:
    explicit HasInfoOptional(Option<std::string> info) : _info(info) {}
    // Запрещаем удаление через укзатель на этот класс
    virtual ~HasInfoOptional() {}
protected:
    Option<std::string> _info;
};

class Namespace;

class NamespaceAware
{
public:
    std::shared_ptr<Namespace> _namespace() const { return __namespace.lock(); }
protected:
    explicit NamespaceAware(const std::weak_ptr<Namespace> & _namespace) : __namespace(_namespace) {}
    virtual ~NamespaceAware() {}
protected:
    const std::weak_ptr<Namespace> __namespace;
};

/*!
 * Класс единицы измерения
 */
class Unit : public HasName, public HasInfoOptional, public NamespaceAware
{
public:
    explicit Unit(const Name &name, const std::weak_ptr<Namespace> & _namespace, const std::string &display, const Option<std::string> &info)
            : HasInfoOptional(info), NamespaceAware(_namespace), _name(name), _display(display) {}
    virtual ~Unit() {}
    const Name &name() const override { return _name; }
    std::string& display() { return _display; }
private:
    Name _name;
    std::string _display;
};

/*
 * Определение визиторов для типов
 */

class PrimitiveType;
class AliasType;
class SubType;
class EnumType;
class StructType;
class ArrayType;

class TypeVisitor
{
public:
    virtual void visit(PrimitiveType & primitiveType) = 0;
    virtual void visit(AliasType & aliasType) = 0;
    virtual void visit(SubType & subType) = 0;
    virtual void visit(EnumType & enumType) = 0;
    virtual void visit(StructType & structType) = 0;
    virtual void visit(ArrayType & arrayType) = 0;
};

class TypeVisitable
{
public:
    virtual void accept(TypeVisitor & typeVisitor) = 0;
};

/*
 * Типы
 */

class ValueOrError
{
public:
    explicit ValueOrError(Variant *value) : _value(value) { BMCL_ASSERT_MSG(value != nullptr, "value must not be nullptr"); }
    explicit ValueOrError(std::unique_ptr<Variant> && value) : _value(std::move(value)) { BMCL_ASSERT_MSG(value.get() != nullptr, "value must not be nullptr"); }
    ValueOrError(ValueOrError& o) = delete;
    ValueOrError(ValueOrError&& o) : _value(std::move(o.value())) {}
    bool isError() const { return _value.get() == nullptr; }
    bool isValue() const { return _value.get() != nullptr; }
    std::unique_ptr<Variant> &value() { return _value; }
    std::unique_ptr<Variant> const &value() const { return _value; }
private:
    std::unique_ptr<Variant> _value;
};

QDebug operator<<(QDebug dbg, const ValueOrError &v);

class Type : public HasNameOptional, public HasInfoOptional, public TypeVisitable, public NamespaceAware
{
public:
    explicit Type(const Option<Name> &name, const std::weak_ptr<Namespace> & _namespace, const Option<std::string> &info)
            : HasInfoOptional(info), NamespaceAware(_namespace), _name(name) {}
    virtual ~Type() {}
    virtual Option<std::string> serialize(const Variant &value, MemWriter &writer) const = 0;
    virtual ValueOrError deserialize(MemReader &reader) const = 0;
    Option<Name> nameOptional() override { return _name; }
    virtual void updateIndices() {}
protected:
    Option<Name> _name;
};

class HasBaseType
{
public:
    explicit HasBaseType(const std::weak_ptr<Type> & baseType) : _baseType(baseType) {}
    std::shared_ptr<Type> baseType() const { return _baseType.lock(); }
protected:
    virtual ~HasBaseType() {}
private:
    std::weak_ptr<Type> _baseType;
};

enum class PrimitiveTypeKind { Float, Uint, Int, Bool };

PrimitiveTypeKind primitiveTypeKindOf(const std::string &value);

class PrimitiveType : public Type
{
public:
    explicit PrimitiveType(const Option<Name> &name, const std::weak_ptr<Namespace> & _namespace, const Option<std::string> &info, PrimitiveTypeKind typeKind, uint_fast8_t bitLength)
            : Type(name, _namespace, info), _typeKind(typeKind), _bitLength(bitLength) { }
    void accept(TypeVisitor & typeVisitor) override { typeVisitor.visit(*this); }
    PrimitiveTypeKind typeKind() { return _typeKind; }
    uint_fast8_t bitLength() { return _bitLength; }
    virtual Option<std::string> serialize(const Variant &value, MemWriter &writer) const override;
    virtual ValueOrError deserialize(MemReader &reader) const override;
private:
    PrimitiveTypeKind _typeKind;
    uint_fast8_t _bitLength;
};

class AliasType : public Type, public HasBaseType
{
public:
    explicit AliasType(const Option<Name> &name, const std::weak_ptr<Namespace> & _namespace, const Option<std::string> &info,
                       const std::weak_ptr<Type> & type)
            : Type(name, _namespace, info), HasBaseType(type) { }
    void accept(TypeVisitor &typeVisitor) override { typeVisitor.visit(*this); }
    virtual Option<std::string> serialize(const Variant &value, MemWriter &writer) const override { return baseType()->serialize(value, writer); }
    virtual ValueOrError deserialize(MemReader &reader) const override { return ValueOrError(std::move(baseType()->deserialize(reader))); }
};

class SubType : public Type, public HasBaseType
{
public:
    SubType(const Option<Name> &name, const std::weak_ptr<Namespace> & _namespace, const Option<std::string> &info,
            const std::weak_ptr<Type> & type)
            : Type(name, _namespace, info), HasBaseType(type) { }
    void accept(TypeVisitor &typeVisitor) override { typeVisitor.visit(*this); }
    virtual Option<std::string> serialize(const Variant &value, MemWriter &writer) const override { return baseType()->serialize(value, writer); }
    virtual ValueOrError deserialize(MemReader &reader) const override { return baseType()->deserialize(reader); }
};

class EnumConstant : public HasName, public HasInfoOptional
{
public:
    explicit EnumConstant(const Name &name, const Option<std::string> &info, const std::string &value)
            : HasInfoOptional(info), _name(name), _value(value) { }
    virtual ~EnumConstant(){}
    const Name &name() const override { return _name; }
private:
    Name _name;
    std::string _value;
};

class EnumType : public Type, public HasBaseType
{
public:
    explicit EnumType(const Option<Name>& name, const std::weak_ptr<Namespace> & _namespace, const Option<std::string>& info,
                      const std::weak_ptr<Type> & baseType, std::vector<std::shared_ptr<EnumConstant>> && constants);
    virtual ~EnumType() override {}
    void accept(TypeVisitor &typeVisitor) override { typeVisitor.visit(*this); }
    virtual Option<std::string> serialize(const Variant &value, MemWriter &writer) const override { return baseType()->serialize(value, writer); }
    virtual ValueOrError deserialize(MemReader &reader) const override { return baseType()->deserialize(reader); }
    std::vector<std::shared_ptr<EnumConstant>> & constants() { return _constants; }
    const std::vector<std::shared_ptr<EnumConstant>> & constants() const { return _constants; }
    virtual void updateIndices() override
    {
        _constantsByName.clear();
        for(auto & constant : _constants)
        {
            _constantsByName.emplace(constant->name(), constant);
        }
    }
private:
    std::vector<std::shared_ptr<EnumConstant>> _constants;
    std::map<std::string, std::weak_ptr<EnumConstant>> _constantsByName;
};

class StructField : public HasName, public HasBaseType, public HasInfoOptional
{
public:
    explicit StructField(const Name &name, const Option<std::string> &info, const std::weak_ptr<Type> & type,
                         const std::weak_ptr<Unit> & unit)
            :  HasBaseType(type), HasInfoOptional(info), _name(name), _unit(unit) { }
    virtual ~StructField(){}
    const Name &name() const override { return _name; }
    const std::shared_ptr<Unit> &unit() { return _unit.lock(); }
private:
    Name _name;
    std::weak_ptr<Unit> _unit;
};

class StructType : public Type
{
public:
    explicit StructType(const Option<Name> &name, const std::weak_ptr<Namespace> & _namespace, const Option<std::string> &info,
                        std::vector<std::shared_ptr<StructField>> && fields)
            : Type(name, _namespace, info), _fields(std::move(fields)) { BMCL_ASSERT(!_fields.empty()); updateIndices(); }
    virtual ~StructType() override {}
    void accept(TypeVisitor &typeVisitor) override { typeVisitor.visit(*this); }
    std::vector<std::shared_ptr<StructField>> & fields() { return _fields; }
    std::shared_ptr<StructField> fieldByName(Name name) { auto it(_fieldsByName.find(name)); return it == _fieldsByName.end() ? std::shared_ptr<StructField>() : it->second.lock(); }
    virtual void updateIndices() override;
    virtual Option<std::string> serialize(const Variant &value, MemWriter &writer) const override;
    virtual ValueOrError deserialize(MemReader &reader) const override;
private:
    std::vector<std::shared_ptr<StructField>> _fields;
    std::map<Name, std::weak_ptr<StructField>> _fieldsByName;
};

class ArrayType : public Type, public HasBaseType
{
public:
    explicit ArrayType(const Option<Name> &name, const std::weak_ptr<Namespace> & _namespace, const Option<std::string> &info,
                       const std::weak_ptr<Type> & baseType,
                       uint_fast64_t minLength, uint_fast64_t maxLength);
    virtual ~ArrayType() {}
    void accept(TypeVisitor &typeVisitor) override { typeVisitor.visit(*this); }
    uint_fast64_t minLength() { return _minLength; }
    uint_fast64_t maxLength() { return _maxLength; }
    bool isFixedSize() const { return _minLength == _maxLength; }
    virtual Option<std::string> serialize(const Variant &value, MemWriter &writer) const override;
    virtual ValueOrError deserialize(MemReader &reader) const override;
private:
    uint_fast64_t _minLength, _maxLength;
};

/*
 * Компоненты и их составляющие -- команды и сообщения
 */

class Component;

class ComponentAware
{
public:
    std::shared_ptr<Component> component() const { return _component.lock(); }
protected:
    explicit ComponentAware(const std::weak_ptr<Component> & component) : _component(component) {}
    virtual ~ComponentAware() {}
private:
    std::weak_ptr<Component> _component;
};

/*
 * Команды
 */

class CommandArgument : public HasName, public HasInfoOptional, public HasBaseType
{
public:
    explicit CommandArgument(const Name &name, const std::weak_ptr<Type> & type, const std::weak_ptr<Unit> &unit,
                             const Option<std::string> & info)
            : HasInfoOptional(info), HasBaseType(type), _name(name), _unit(unit) {}
    virtual ~CommandArgument(){}
    const Name &name() const override { return _name; }
    std::shared_ptr<Unit> unit() { return _unit.lock(); }
private:
    Name _name;
    std::weak_ptr<Unit> _unit;
};

class Command : public HasName, public ComponentAware
{
public:
    explicit Command(const Name &name, uint_fast32_t id, const std::weak_ptr<Component> & component,
                     std::vector<std::shared_ptr<CommandArgument>> && arguments)
            : ComponentAware(component), _name(name), _id(id), _arguments(std::move(arguments)) {}
    virtual ~Command() {}
    Option<std::string> serialize(MapVariant &map, MemWriter &writer) const;
    const Name &name() const override { return _name; }
    uint_fast32_t id() { return _id; }
    std::vector<std::shared_ptr<CommandArgument>> & arguments() { return _arguments; }
private:
    Name _name;
    uint_fast32_t _id;
    std::vector<std::shared_ptr<CommandArgument>> _arguments;
};

/*
 * Сообщения
 */

class MessageParameter
{
public:
    explicit MessageParameter(const std::string &value) : _value(value) {}
    const std::string& value() const { return _value; }
private:
    std::string _value;
};

class Message : public HasName, public ComponentAware
{
public:
    virtual ~Message(){}
    virtual std::vector<std::shared_ptr<MessageParameter>> & parameters() = 0;
    virtual const std::vector<std::shared_ptr<MessageParameter>> & parameters() const = 0;
    const Name &name() const override { return _name; }
    uint_fast32_t id() { return _id; }
    Option<std::string> serialize(MapVariant &map, MemWriter &writer) const;
    ValueOrError deserialize(MemReader &reader) const;
protected:
    explicit Message(const Name &name, uint_fast32_t id, const std::weak_ptr<Component> & component)
            : ComponentAware(component), _name(name), _id(id) {}
    std::shared_ptr<Type> findTypeForParameter(const MessageParameter &parameter) const;
protected:
    Name _name;
    uint_fast32_t _id;
};

class AbstractMessage : public Message
{
public:
    std::vector<std::shared_ptr<MessageParameter>> & parameters() { return _parameters; }
    const std::vector<std::shared_ptr<MessageParameter>> & parameters() const { return _parameters; }

protected:
    explicit AbstractMessage(const Name &name, uint_fast32_t id, const std::weak_ptr<Component> & component,
                             std::vector<std::shared_ptr<MessageParameter>> && parameters = std::vector<std::shared_ptr<MessageParameter>>())
            : Message(name, id, component), _parameters(std::move(parameters)) {}
    virtual ~AbstractMessage() {}
protected:
    std::vector<std::shared_ptr<MessageParameter>> _parameters;
};

class StatusMessage : public AbstractMessage
{
public:
    explicit StatusMessage(const Name &name, uint_fast32_t id, const std::weak_ptr<Component> & component,
                           std::vector<std::shared_ptr<MessageParameter>> && parameters = std::vector<std::shared_ptr<MessageParameter>>())
            : AbstractMessage(name, id, component, std::move(parameters)) {}
};

class EventMessage : public AbstractMessage
{
public:
    explicit EventMessage(const Name &name, uint_fast32_t id, const std::weak_ptr<Component> & component,
                          std::vector<std::shared_ptr<MessageParameter>> && parameters = std::vector<std::shared_ptr<MessageParameter>>())
            : AbstractMessage(name, id, component, std::move(parameters)) {}
};

class DynamicStatusMessage : public AbstractMessage
{
public:
    explicit DynamicStatusMessage(const Name &name, uint_fast32_t id, const std::weak_ptr<Component> & component,
                                  std::vector<std::shared_ptr<MessageParameter>> && parameters = std::vector<std::shared_ptr<MessageParameter>>())
            : AbstractMessage(name, id, component, std::move(parameters)) {}
};

class ComponentComparator;

typedef std::set<Component*, ComponentComparator> UniqueOrderedComponentSet;

class Component : public HasName, public HasBaseType, public NamespaceAware, public HasInfoOptional
{
public:
    explicit Component(const Name &name, const std::weak_ptr<Namespace> & _namespace,
                       const std::weak_ptr<Type> & baseType, const Option<std::string> &info,
                       std::vector<std::weak_ptr<Component>> && components = std::vector<std::weak_ptr<Component>>(),
                       std::vector<std::shared_ptr<Command>> && commands = std::vector<std::shared_ptr<Command>>(),
                       std::vector<std::shared_ptr<Message>> && messages = std::vector<std::shared_ptr<Message>>())
            : NamespaceAware(_namespace), HasInfoOptional(info), HasBaseType(baseType), _name(name)
            , _subComponents(std::move(components)), _commands(std::move(commands)), _messages(std::move(messages)) {}
    virtual ~Component() {}
    const Name &name() const override { return _name; }
    std::vector<std::weak_ptr<Component>> & subComponents() { return _subComponents; }
    std::vector<std::shared_ptr<Command>> & commands() { return _commands; }
    std::vector<std::shared_ptr<Message>> & messages() { return _messages; }
    std::string fqn() const;

    std::shared_ptr<Component> subComponentByName(const Name &name) { auto it(_subComponentsByName.find(name)); return it == _subComponentsByName.end() ? std::shared_ptr<Component>() : it->second.lock(); }
    std::shared_ptr<Message> messageByName(const Name &name) { auto it(_messagesByName.find(name)); return it == _messagesByName.end() ? std::shared_ptr<Message>() : it->second.lock(); }
    std::shared_ptr<Message> messageById(uint_fast16_t id) { auto it(_messagesById.find(id)); return it == _messagesById.end() ? std::shared_ptr<Message>() : it->second.lock(); }
    std::shared_ptr<Message> messageById(uint_fast16_t id) const { auto it(_messagesById.find(id)); return it == _messagesById.end() ? std::shared_ptr<Message>() : it->second.lock(); }
    std::shared_ptr<Command> commandByName(const Name &name) { auto it(_commandsByName.find(name)); return it == _commandsByName.end() ? std::shared_ptr<Command>() : it->second.lock(); }
    std::shared_ptr<Command> commandById(uint_fast16_t id) const { auto it(_commandsById.find(id)); return it == _commandsById.end() ? std::shared_ptr<Command>() : it->second.lock(); }
    UniqueOrderedComponentSet uniqueOrderedSubComponentSet();
    void updateIndices();
private:
    Name _name;
    std::vector<std::weak_ptr<Component>> _subComponents;
    std::vector<std::shared_ptr<Command>> _commands;
    std::vector<std::shared_ptr<Message>> _messages;
    std::map<Name, std::weak_ptr<Component>> _subComponentsByName;
    std::map<Name, std::weak_ptr<Command>> _commandsByName;
    std::map<Name, std::weak_ptr<Message>> _messagesByName;
    std::map<uint_fast16_t, std::weak_ptr<Message>> _messagesById;
    std::map<uint_fast16_t, std::weak_ptr<Command>> _commandsById;
    void appendSubComponentsToSet(UniqueOrderedComponentSet &set);
};

/*
 * Пространство имен
 */

class Namespace : public HasName
{
public:
    explicit Namespace(const Name &name, const std::weak_ptr<Namespace> &parentNamespace,
                       std::vector<std::shared_ptr<Namespace>> && subNamespaces = std::vector<std::shared_ptr<Namespace>>(),
                       std::vector<std::shared_ptr<Unit>> && units = std::vector<std::shared_ptr<Unit>>(),
                       std::vector<std::shared_ptr<Type>> && types = std::vector<std::shared_ptr<Type>>(),
                       std::vector<std::shared_ptr<Component>> && components = std::vector<std::shared_ptr<Component>>())
            : _name(name), _parentNamespace(parentNamespace), _subNamespaces(subNamespaces), _units(units), _types(types)
            , _components(std::move(components)) {}
    virtual ~Namespace() {}
    const Name &name() const override { return _name; }
    std::shared_ptr<const Namespace> parentNamespace() { return _parentNamespace.lock(); }
    void setParentNamespace(const std::weak_ptr<const Namespace> & parentNamespace) { _parentNamespace = parentNamespace; }
    std::vector<std::shared_ptr<Namespace>> & subNamespaces() { return _subNamespaces; }
    std::string fqn() const { if (auto s = _parentNamespace.lock()) return s->fqn() + std::string(".") + _name; else return _name; }
    void updateIndices();
    std::vector<std::shared_ptr<Unit>> & units() { return _units; }
    const std::vector<std::shared_ptr<Unit>> & units() const { return _units; }
    std::vector<std::shared_ptr<Type>> & types() { return _types; }
    const std::vector<std::shared_ptr<Type>> & types() const { return _types; }
    std::vector<std::shared_ptr<Component>> & components() { return _components; }
    const std::vector<std::shared_ptr<Component>> & components() const { return _components; }
    std::shared_ptr<Namespace> subNamespaceByName(const Name &name) { auto it(_subNamespaceByName.find(name)); return it == _subNamespaceByName.end() ? std::shared_ptr<Namespace>() : it->second.lock(); }
    std::shared_ptr<Component> componentByName(const Name &name) { auto it(_componentByName.find(name)); return it == _componentByName.end() ? std::shared_ptr<Component>() : it->second.lock(); }
private:
    Name _name;
    std::weak_ptr<const Namespace> _parentNamespace;
    std::vector<std::shared_ptr<Namespace>> _subNamespaces;
    std::vector<std::shared_ptr<Unit>> _units;
    std::vector<std::shared_ptr<Type>> _types;
    std::vector<std::shared_ptr<Component>> _components;
    std::map<Name, std::weak_ptr<Namespace>> _subNamespaceByName;
    std::map<Name, std::weak_ptr<Unit>> _unitByName;
    std::map<Name, std::weak_ptr<Type>> _typeByName;
    std::map<Name, std::weak_ptr<Component>> _componentByName;
};

class ComponentComparator
{
public:
    bool operator()(const Component* left, const Component* right)
    {
        return left->fqn() < right->fqn();
    }
};

/*
 * Реестр моделей
 */

class Registry
{
public:
    explicit Registry(std::vector<std::shared_ptr<Namespace>> && rootNamespaces);
    virtual ~Registry() {}
    std::vector<std::shared_ptr<Namespace>> & rootNamespaces() { return _rootNamespaces; }
    std::shared_ptr<Namespace> rootNamespaceByName(Name name) { auto it(_rootNamespacesByName.find(name)); return it == _rootNamespacesByName.end() ? std::shared_ptr<Namespace>() : it->second.lock(); }
    std::shared_ptr<Message> messageByFqn(const std::string &fqn);
    std::shared_ptr<Command> commandByFqn(const std::string &fqn);
    std::shared_ptr<Component> componentByFqn(const std::string &fqn);
    std::shared_ptr<Namespace> namespaceByFqn(const std::string &fqn);
private:
    std::vector<std::shared_ptr<Namespace>> _rootNamespaces;
    std::map<Name, std::weak_ptr<Namespace>> _rootNamespacesByName;
};

class AsArrayTypeVisitor : public TypeVisitor
{
public:
    virtual void visit(PrimitiveType & primitiveType) { BMCL_UNUSED(primitiveType); }
    virtual void visit(AliasType & aliasType) { aliasType.baseType()->accept(*this); }
    virtual void visit(SubType & subType) { subType.baseType()->accept(*this); }
    virtual void visit(EnumType & enumType) { BMCL_UNUSED(enumType); }
    virtual void visit(StructType & structType) { BMCL_UNUSED(structType);}
    virtual void visit(ArrayType & arrayType) { _result = &arrayType; }
    const ArrayType & result() const { return *_result; }
    ArrayType & result() { return *_result; }
    bool hasResult() const { return _result != nullptr; }
private:
    ArrayType * _result;
};

}
}