/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#pragma once

#include <cstdint>
#include <vector>
#include <cstring>
#include <cassert>

namespace mcc {
namespace s57 {

struct FieldDescription {
    char name[5];
    uint32_t length;
    uint32_t offset;

    bool nameIs(const char* str) const
    {
        return std::strcmp(str, name) == 0;
    }
};

class Directory {
public:
    Directory();
    void addField(const FieldDescription& field);
    void reserveFields(std::size_t size);
    std::size_t fieldNum() const;
    bool hasFields() const;
    bool isEmpty() const;
    const FieldDescription& currentField() const;
    const FieldDescription& fieldAt(std::size_t index) const;
    void popFront();

private:
    std::vector<FieldDescription> _fields;
    std::size_t _currentIndex;
};

inline Directory::Directory()
    : _currentIndex(0)
{
}

inline void Directory::reserveFields(std::size_t size)
{
    _fields.reserve(size);
}

inline std::size_t Directory::fieldNum() const
{
    assert(_currentIndex <= _fields.size());
    return _fields.size() - _currentIndex;
}

inline void Directory::addField(const FieldDescription& field)
{
    _fields.push_back(field);
}

inline const FieldDescription& Directory::currentField() const
{
    assert(_currentIndex < _fields.size());
    return _fields[_currentIndex];
}

inline const FieldDescription& Directory::fieldAt(std::size_t index) const
{
    assert((_currentIndex + index) < _fields.size());
    return _fields[_currentIndex + index];
}

inline bool Directory::hasFields() const
{
    return !isEmpty();
}

inline bool Directory::isEmpty() const
{
    return _currentIndex == _fields.size();
}

inline void Directory::popFront()
{
    _currentIndex++;
    assert(_currentIndex <= _fields.size());
}
}
}
