/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#pragma once

#include <mcc/misc/Option.h>
#include <mcc/misc/Result.h>

#include <atomic>
#include <condition_variable>
#include <deque>
#include <mutex>
#include <memory>
#include <chrono>

namespace mcc {
namespace misc {

enum class ChannelError { Empty, Closed, Timeout };

template <typename T>
class Channel {
public:
    Channel();
    ~Channel();

    template <typename... A>
    bool sendEmplace(A&&... args);
    bool send(const T& value);
    bool send(T&& value);
    mcc::misc::Option<T> recv();
    mcc::misc::Result<T, ChannelError> tryRecv();
    template <typename R, typename P>
    mcc::misc::Result<T, ChannelError> tryRecvFor(const std::chrono::duration<R, P>& dur);
    void clear();
    bool isEmpty() const;
    std::size_t size() const;
    void close();
    void reopen();
    bool isOpen() const { return _isOpen; }

    Channel(const Channel&) = delete;
    Channel(Channel&&) = delete;
    Channel& operator=(const Channel&) = delete;
    Channel& operator=(Channel&&) = delete;

private:
    std::deque<T> _queue;
    mutable std::mutex _queueMutex;
    std::condition_variable _queueNotEmpty;
    std::atomic<bool> _isOpen;
};

template <typename T>
inline Channel<T>::Channel()
    : _isOpen(true)
{
}

template <typename T>
inline Channel<T>::~Channel()
{
    close();
}

template <typename T>
bool Channel<T>::send(const T& value)
{
    std::lock_guard<std::mutex> lock(_queueMutex);
    _queue.push_back(value);
    _queueNotEmpty.notify_one();
    return _isOpen;
}

template <typename T>
bool Channel<T>::send(T&& value)
{
    std::lock_guard<std::mutex> lock(_queueMutex);
    _queue.push_back(std::forward<T>(value));
    _queueNotEmpty.notify_one();
    return _isOpen;
}

template <typename T>
template <typename... A>
bool Channel<T>::sendEmplace(A&&... args)
{
    std::lock_guard<std::mutex> lock(_queueMutex);
    _queue.emplace_back(std::forward<A>(args)...);
    _queueNotEmpty.notify_one();
    return _isOpen;
}

template <typename T>
mcc::misc::Option<T> Channel<T>::recv()
{
    std::unique_lock<std::mutex> lock(_queueMutex);
    bool isActive;
    _queueNotEmpty.wait(lock, [&]() {
        isActive = _isOpen;
        return !_queue.empty() || !isActive;
    });
    if (!isActive) {
        return None;
    }
    mcc::misc::Option<T> res(std::move(_queue.front()));
    _queue.pop_front();
    return res;
}

template <typename T>
mcc::misc::Result<T, ChannelError> Channel<T>::tryRecv()
{
    if (!_isOpen) {
        return ChannelError::Closed;
    }
    std::lock_guard<std::mutex> lock(_queueMutex);
    if (_queue.empty()) {
        return ChannelError::Empty;
    }
    mcc::misc::Result<T, ChannelError> res(std::move(_queue.front()));
    _queue.pop_front();
    return res;
}

template <typename T>
template <typename R, typename P>
mcc::misc::Result<T, ChannelError> Channel<T>::tryRecvFor(const std::chrono::duration<R, P>& dur)
{
    std::unique_lock<std::mutex> lock(_queueMutex);
    bool isActive;
    bool hasData = _queueNotEmpty.wait_for(lock, dur, [&]() {
        isActive = _isOpen;
        return !_queue.empty() || !isActive;
    });
    if (!isActive) {
        return ChannelError::Closed;
    }
    if (!hasData) {
        return ChannelError::Timeout;
    }
    mcc::misc::Result<T, ChannelError> res(std::move(_queue.front()));
    _queue.pop_front();
    return res;
}

template <typename T>
inline void Channel<T>::clear()
{
    std::lock_guard<std::mutex> lock(_queueMutex);
    _queue.clear();
}

template <typename T>
inline bool Channel<T>::isEmpty() const
{
    std::lock_guard<std::mutex> lock(_queueMutex);
    return _queue.empty();
}

template <typename T>
inline std::size_t Channel<T>::size() const
{
    std::lock_guard<std::mutex> lock(_queueMutex);
    return _queue.size();
}

template <typename T>
void Channel<T>::close()
{
    _isOpen = false;
    clear();
    _queueNotEmpty.notify_all();
}

template <typename T>
void Channel<T>::reopen()
{
    _isOpen = true;
}

template <typename T>
using ChannelPtr = std::shared_ptr<Channel<T>>;

template <typename T>
class Sender {
public:
    Sender()
    {
    }

    Sender(const ChannelPtr<T>& chan);

    template <typename... A>
    bool sendEmplace(A&&... args);
    bool send(const T& value);
    bool send(T&& value);

    void close();
    void reopen();
    bool isOpen() const { return _chan->isOpen(); }

private:
    ChannelPtr<T> _chan;
};

template <typename T>
inline Sender<T>::Sender(const ChannelPtr<T>& chan)
    : _chan(chan)
{
}

template <typename T>
template <typename... A>
inline bool Sender<T>::sendEmplace(A&&... args)
{
    return _chan->sendEmplace(std::forward<A>(args)...);
}

template <typename T>
inline bool Sender<T>::send(const T& value)
{
    return _chan->send(value);
}

template <typename T>
inline bool Sender<T>::send(T&& value)
{
    return _chan->send(std::forward<T>(value));
}

template <typename T>
inline void Sender<T>::close()
{
    return _chan->close();
}

template <typename T>
inline void Sender<T>::reopen()
{
    return _chan->reopen();
}

template <typename T>
class Reciever {
public:
    Reciever()
    {
    }

    Reciever(const ChannelPtr<T>& chan);

    mcc::misc::Option<T> recv();
    mcc::misc::Result<T, ChannelError> tryRecv();
    template <typename R, typename P>
    mcc::misc::Result<T, ChannelError> tryRecvFor(const std::chrono::duration<R, P>& dur);

private:
    ChannelPtr<T> _chan;
};

template <typename T>
inline Reciever<T>::Reciever(const ChannelPtr<T>& chan)
    : _chan(chan)
{
}

template <typename T>
inline Option<T> Reciever<T>::recv()
{
    return _chan->recv();
}

template <typename T>
inline Result<T, ChannelError> Reciever<T>::tryRecv()
{
    return _chan->tryRecv();
}

template <typename T>
template <typename R, typename P>
inline Result<T, ChannelError> Reciever<T>::tryRecvFor(const std::chrono::duration<R, P>& dur)
{
    return _chan->tryRecvFor(dur);
}

template <typename T, typename R = T>
struct ChannelPair {
    ChannelPair()
    {
    }

    ChannelPair(const ChannelPtr<T>& left)
        : sender(left)
        , reciever(left)
    {
    }

    ChannelPair(const ChannelPtr<T>& left, const ChannelPtr<R>& right)
        : sender(left)
        , reciever(right)
    {
    }

    Sender<T> sender;
    Reciever<R> reciever;
};

template <typename T>
inline ChannelPair<T> makeChannel()
{
    ChannelPtr<T> chan = std::make_shared<Channel<T>>();
    return ChannelPair<T>(chan);
}

template <typename T, typename R = T>
struct BiChannelPair {
    BiChannelPair()
    {
    }

    BiChannelPair(const ChannelPtr<T>& forwardChan, const ChannelPtr<R>& backwardChan)
        : forward(forwardChan, backwardChan)
        , backward(backwardChan, forwardChan)
    {
    }

    ChannelPair<T, R> forward;
    ChannelPair<R, T> backward;
};

template <typename T, typename R = T>
inline BiChannelPair<T, R> makeBiChannel()
{
    ChannelPtr<T> forward = std::make_shared<Channel<T>>();
    ChannelPtr<R> backward = std::make_shared<Channel<R>>();
    return BiChannelPair<T, R>(forward, backward);
}
}
}
