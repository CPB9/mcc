/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "mcc/messages/LocalRouter.h"
#include "mcc/messages/Message.h"
#include "mcc/messages/MessageSender.h"


namespace mcc {
namespace messages {

LocalRouter::LocalRouter() : _isLocked(false), _in(std::make_shared<mcc::messages::MessageQueue::element_type>())
{
}

std::vector<std::string> LocalRouter::locals() const
{
    std::vector<std::string> clients;
    for (const auto i : _queueByName)
    {
        clients.push_back(i.first);
    }
    return clients;
}

void LocalRouter::add(const std::string& client)
{
    if (_isLocked)
    {
        assert(false);
        return;
    }
    auto i = _queueByName.find(client);
    if (i == _queueByName.end())
        _queueByName[client] = std::make_shared<mcc::messages::MessageQueue::element_type>();
}

mcc::messages::MessageQueue LocalRouter::recv(const std::string& client) const
{
    lock();
    const auto i = _queueByName.find(client);
    if (i == _queueByName.end())
    {
        assert(false);
        return mcc::messages::MessageQueue();
    }
    return i->second;
}

mcc::messages::MessageSender LocalRouter::send(const std::string& client) const
{
    lock();
    return std::make_shared<mcc::messages::MessageSenderX>(client, _in);
}


}
}
