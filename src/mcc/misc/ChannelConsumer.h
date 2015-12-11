/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#pragma once

#include <mcc/misc/Channel.h>

#include <atomic>
#include <mutex>
#include <thread>
#include <iostream>

namespace mcc {
namespace misc {

class ChannelConsumer {
public:
    ChannelConsumer();
    ~ChannelConsumer();

    template <typename T, typename C>
    void addWorker(mcc::misc::Channel<T>* channel, C&& callable);
    void joinAll();
    void clear();

    ChannelConsumer(const mcc::misc::ChannelConsumer&) = delete;
    ChannelConsumer(mcc::misc::ChannelConsumer&&) = delete;
    mcc::misc::ChannelConsumer& operator=(const mcc::misc::ChannelConsumer&) = delete;
    mcc::misc::ChannelConsumer& operator=(mcc::misc::ChannelConsumer&&) = delete;

private:
    std::vector<std::thread> _workers;
};

inline ChannelConsumer::ChannelConsumer()
{
}

inline ChannelConsumer::~ChannelConsumer()
{
    joinAll();
}

template <typename T, typename C>
void ChannelConsumer::addWorker(mcc::misc::Channel<T>* channel, C&& callable)
{
    auto lambda = [=]() mutable {
        while (true) {
            mcc::misc::Option<T> data = channel->recv();
            if (data.isNone())
            {
                break;
            }
            callable(data.unwrap());
        }
    };
    _workers.emplace_back(lambda);
}

inline void ChannelConsumer::joinAll()
{
    for (std::thread& worker : _workers) {
        worker.detach();
    }
}

inline void ChannelConsumer::clear()
{
    joinAll();
    _workers.clear();
}
}
}
