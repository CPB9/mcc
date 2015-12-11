/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#pragma once
#include <string>
#include <type_traits>
#include "bmcl/Option.h"
#include "mcc/misc/Helpers.h"
#include "mcc/messages/Deaclarations.h"

namespace mcc {
namespace messages {


class MessageSenderX
{
public:
    MessageSenderX(const std::string& name, const MessageQueue& queue);

    template<class T>
    MessageId send(std::unique_ptr<T>&& message);
    template<typename T, typename... A>
    MessageId send(A&&... args);
    MessageId send(MessagePtr&& message);

    template<class T>
    MessageId sendTo(const std::string& to, std::unique_ptr<T>&& message);
    template<typename T, typename... A>
    MessageId sendTo(const std::string& to, A&&... args);
    MessageId sendTo(const std::string& to, MessagePtr&& message);

    template<class T>
    MessageId respond(std::unique_ptr<T>&& message);
    template<typename T, typename... A>
    MessageId respond(A&&... args);

private:
    static std::atomic<MessageId> _counter;
    std::string _name;
    MessageQueue _queue;
};

template<class T>
MessageId MessageSenderX::send(std::unique_ptr<T>&& message)
{
    static_assert(std::is_base_of<MessageTo, T>::value, "method is used for predefined receivers only");
    MessagePtr p;
    p.reset(static_cast<Message*>(message.release()));
    return send(std::move(p));
}

template<typename T, typename... A>
MessageId MessageSenderX::send(A&&... args)
{
    return send(mcc::misc::makeUnique<T>(std::forward<A>(args)...));
}

template<class T>
MessageId MessageSenderX::sendTo(const std::string& to, std::unique_ptr<T>&& message)
{
    static_assert(std::is_base_of<Message, T>::value, "only messages can be passed on");
    MessagePtr p;
    p.reset(static_cast<Message*>(message.release()));
    return sendTo(to, std::move(p));
}

template<typename T, typename... A>
MessageId MessageSenderX::sendTo(const std::string& to, A&&... args)
{
    return sendTo(to, mcc::misc::makeUnique<T>(std::forward<A>(args)...));
}

template<class T>
MessageId MessageSenderX::respond(std::unique_ptr<T>&& message)
{
    static_assert(std::is_base_of<Response, T>::value, "only responses can be passed on");
    MessagePtr p;
    p.reset(static_cast<Message*>(message.release()));
    return send(std::move(p));
}

template<typename T, typename... A>
MessageId MessageSenderX::respond(A&&... args)
{
    return respond(mcc::misc::makeUnique<T>(std::forward<A>(args)...));
}



}
}

