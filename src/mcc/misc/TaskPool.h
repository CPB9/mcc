/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#pragma once

#include <mcc/misc/Channel.h>

#include <thread>
#include <vector>

namespace mcc {
namespace misc {

template <typename F>
class TaskPool {
public:
    TaskPool(std::size_t nThreads);
    TaskPool();
    ~TaskPool();

    void start(std::size_t nThreads);
    void stop();
    bool schedule(const F& task);
    bool schedule(F&& task);
    void clear();

private:
    void mainLoop();

    mcc::misc::Channel<F> _tasks;
    std::vector<std::thread> _threads;
};

template <typename F>
inline TaskPool<F>::TaskPool(std::size_t nThreads)
{
    start(nThreads);
}

template <typename F>
inline TaskPool<F>::TaskPool()
{
    start(std::thread::hardware_concurrency());
}

template <typename F>
inline TaskPool<F>::~TaskPool()
{
    stop();
}

template <typename F>
void TaskPool<F>::start(std::size_t nThreads)
{
    assert(_threads.empty());
    for (std::size_t i = 0; i < nThreads; i++) {
        _threads.emplace_back(&TaskPool<F>::mainLoop, this);
    }
}

template <typename F>
void TaskPool<F>::stop()
{
    _tasks.close();
    for (std::thread& thread : _threads) {
        thread.join();
    }
    _threads.clear();
}

template <typename F>
inline bool TaskPool<F>::schedule(const F& task)
{
    return _tasks.send(task);
}

template <typename F>
inline bool TaskPool<F>::schedule(F&& task)
{
    return _tasks.send(std::move(task));
}

template <typename F>
inline void TaskPool<F>::clear()
{
    _tasks.clear();
}

template <typename F>
void TaskPool<F>::mainLoop()
{
    while (true) {
        mcc::misc::Option<F> task = _tasks.recv();
        if (!task.isSome()) {
            return;
        }
        task.unwrap()();
    }
}
}
}
