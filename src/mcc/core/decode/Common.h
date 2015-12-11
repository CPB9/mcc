/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#pragma once

#include "bmcl/Assert.h"

namespace mcc
{
namespace decode
{

template<typename T>
class OptionPtr
{
public:
    explicit OptionPtr() : _ptr(nullptr) {}
    explicit OptionPtr(T* ptr) : _ptr(ptr) {}
    bool isSome() const { return _ptr != nullptr; }
    bool isNone() const { return _ptr == nullptr; }
    T* get() { return _ptr; }
    const T* get() const { return _ptr; }
    void reset(T* ptr) { _ptr = ptr; }
    T *operator->() { BMCL_ASSERT_MSG(_ptr != nullptr, "-> on empty OptionPtr"); return _ptr; }
    const T *operator->() const { BMCL_ASSERT_MSG(_ptr != nullptr, "-> on empty OptionPtr"); return _ptr; }
    T &operator*() { BMCL_ASSERT_MSG(_ptr != nullptr, "operator* on empty OptionPtr"); return *_ptr; }
    T* orElse(T* alternative) { return _ptr == nullptr ? alternative : _ptr; }
    const T* orElse(const T* alternative) const { return _ptr == nullptr ? alternative : _ptr; }
    operator bool() const { return _ptr != nullptr; }
private:
    T* _ptr;
};
}
}