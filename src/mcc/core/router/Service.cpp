/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include <QCoreApplication>
#include <QDebug>
#include <vector>

#include "mcc/Names.h"
#include "mcc/messages/MessageSender.h"
#include "mcc/core/router/Service.h"


namespace mcc {
namespace core {
namespace router {

Service::Service(const mcc::messages::LocalRouterPtr& router) : mcc::messages::ServiceAbstract(mcc::Names::coreRouter(), router)
{
    _out = nullptr;
    _in = nullptr;

    _start = std::chrono::steady_clock::now();
}

Service::~Service()
{
}

void Service::tick()
{
    if (std::chrono::duration_cast<std::chrono::seconds>(std::chrono::steady_clock::now() - _start).count() >= 10)
    {
        for (const auto& queue : _router->_queueByName)
        {
            if (queue.second->size() > 100)
                qDebug() << QString::fromStdString(queue.first) << queue.second->size();
        }
        _start = std::chrono::steady_clock::now();
    }

    auto m = _router->_in->tryRecvFor(std::chrono::seconds(1));
    if (m.isErr())
        return;

    routeMessage_(m.take());
}

void Service::sendMulticast(const std::string& from, mcc::messages::MessagePtr&& message)
{
    for (const auto& j : _router->_queueByName)
    {
        if (j.first == from)
            continue;
        auto clone = message->clone();
        clone->_receiver = j.first;
        j.second->send(std::move(clone));
    }
}

void Service::routeMessage_(std::unique_ptr<mcc::messages::Message>&& message)
{
    const std::string& receiver = message->receiver();
    const std::string& sender = message->sender();

    if (message->receiver() == mcc::Names::multicast())
    {
        sendMulticast(sender, std::move(message));
        return;
    }

    auto to = _router->_queueByName.find(receiver);
    if (to == _router->_queueByName.end())
    {
        if (sender != name())
        {
            auto& queue = _router->_queueByName[sender];
            queue->send(std::unique_ptr<mcc::messages::Message>(new mcc::messages::Error(message.get(), "destination unknown: " + receiver)));
        }
        return;
    }

    to->second->send(std::move(message));
}

}
}
}