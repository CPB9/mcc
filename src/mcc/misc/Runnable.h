/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#pragma once
#include <mutex>
#include <future>
#include <string>
#include <condition_variable>
#include <QThread>
#include <QDebug>

namespace mcc {
namespace misc {

class Runnable
{
public:
    virtual ~Runnable() { assert(!isRunning_()); }
    void finish(bool wait = false);
    const std::string& name() const { return _name; }
    bool isFinished() const;

    template<typename T, typename... A>
    static std::unique_ptr<T> startInThread(A&&... args);

protected:
    Runnable(const std::string& name) : _name(name) { _isRunning = true; }
    bool isRunning_() const { return _isRunning; }
    void stopRunning_() { _isRunning = false; }
    virtual bool pre() = 0;
    virtual void tick() = 0;
    virtual void post() = 0;

    mutable std::mutex      _mutex;
    std::condition_variable _wakeup;

private:
    void run();

    std::string                 _name;
    mutable std::future<void>   _future;
    std::atomic<bool>           _isRunning;
};

template<typename T, typename... A>
inline std::unique_ptr<T> Runnable::startInThread(A&&... args)
{
    auto ptr = std::unique_ptr<T>(new T(std::forward<A>(args)...));
    ptr->_future = std::async(std::launch::async, &Runnable::run, ptr.get());
    return ptr;
}

inline void Runnable::finish(bool wait)
{
    if (isFinished())
        return;
    _isRunning = false;
    _wakeup.notify_all();
    if (!wait)
        return;
    if (_future.valid())
    {
        if (std::future_status::ready == _future.wait_for(std::chrono::seconds(10)))
        {
            _future.get();
            qDebug() << "thread destroyed";
        }
        else
        {
            qDebug() << "stuck";
        }
    }
}

inline bool Runnable::isFinished() const
{
    if (_isRunning)
        return false;
    if (!_future.valid())
        return true;
    if (std::future_status::ready == _future.wait_for(std::chrono::microseconds(0)))
    {
        _future.get();
        return true;
    }
    return false;
}

inline void Runnable::run()
{
    try
    {
        QThread::currentThread()->setObjectName(QString::fromStdString(_name));
        qDebug() << "started";

        if (!pre())
        {
            post();
            return;
        }

        qDebug() << "inited";

        while (_isRunning)
        {
            tick();
        }

        post();
        qDebug() << "finished";
    }
    catch (std::exception& e)
    {
        Q_UNUSED(e);

        assert(false);
    }
    catch (...)
    {
        assert(false);
    }
}
}
}