/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#pragma once

#include <condition_variable>
#include <memory>
#include <mutex>

namespace mcc {
namespace misc {

template <typename T>
class SharedVar;

template <typename T>
class SharedVar {
public:
    template <typename... A>
    SharedVar(A&&... args)
        : _var(std::forward<A>(args)...)
        , _hasData(false)
    {
    }

    // TODO: make friend
    std::condition_variable _readCondition;
    std::condition_variable _writeCondition;
    std::mutex _mutex;
    T _var;
    bool _hasData;
};

template <typename T>
using SharedVarPtr = std::shared_ptr<SharedVar<T>>;

template <typename T, typename... A>
SharedVarPtr<T> makeSharedVar(A&&... args)
{
    return std::make_shared<SharedVar<T>>(std::forward<A>(args)...);
}

template <typename T, std::condition_variable SharedVar<T>::*firstLock,
          std::condition_variable SharedVar<T>::*secondLock, bool hasData>
class SharedVarLock {
public:
    SharedVarLock(SharedVar<T>& var)
        : _sharedVar(var)
        , _lock(var._mutex)
    {
        (_sharedVar.*firstLock).wait(_lock, [this]() { return _sharedVar._hasData == hasData; });
    }

    SharedVarLock(const SharedVarLock&) = delete;

    SharedVarLock(SharedVarLock&& other)
        : _sharedVar(other._sharedVar)
        , _lock(std::move(other._lock))
    {
    }

    ~SharedVarLock()
    {
        _sharedVar._hasData = !hasData;
        (_sharedVar.*secondLock).notify_one();
    }

    const T* operator->() const
    {
        return &_sharedVar._var;
    }

    T* operator->()
    {
        return &_sharedVar._var;
    }

    const T& operator*() const
    {
        return _sharedVar._var;
    }

    T& operator*()
    {
        return _sharedVar._var;
    }

    const T& value() const
    {
        return _sharedVar._var;
    }

    T& value()
    {
        return _sharedVar._var;
    }

    void setValue(const T& value)
    {
        _sharedVar._var = value;
    }

    void setValue(T&& value)
    {
        _sharedVar._var = value;
    }

    SharedVarLock& operator=(const SharedVarLock&) = delete;
    SharedVarLock& operator=(SharedVarLock&&) = delete;

private:
    SharedVar<T>& _sharedVar;
    std::unique_lock<std::mutex> _lock;
};

template <typename T>
using SharedVarReaderLock = SharedVarLock<T, &SharedVar<T>::_readCondition, &SharedVar<T>::_writeCondition, true>;

template <typename T>
using SharedVarWriterLock = SharedVarLock<T, &SharedVar<T>::_writeCondition, &SharedVar<T>::_readCondition, false>;

template <typename T, typename L>
class SharedVarAccessor {
public:
    SharedVarAccessor(const std::shared_ptr<SharedVar<T>>& var)
        : _var(var)
    {
    }

    L lock()
    {
        return L(*_var);
    }

private:
    std::shared_ptr<SharedVar<T>> _var;
};

template <typename T>
using SharedVarReader = SharedVarAccessor<T, SharedVarReaderLock<T>>;

template <typename T>
using SharedVarWriter = SharedVarAccessor<T, SharedVarWriterLock<T>>;
}
}
