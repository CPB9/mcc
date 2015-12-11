/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include <QDebug>
#include "bmcl/MemReader.h"
#include "bmcl/MemWriter.h"
#include "mcc/misc/TimeUtils.h"
#include "mcc/messages/MessageSender.h"
#include "mcc/messages/Message.h"


namespace mcc {
namespace messages {

std::atomic<MessageId> MessageSenderX::_counter(0);

MessageSenderX::MessageSenderX(const std::string& name, const MessageQueue& queue) :_name(name), _queue(queue)
{
}

MessageId MessageSenderX::send(MessagePtr&& message)
{
    message->_sender = _name;
    if (message->receiver().empty())
    {
        qDebug() << QString::fromStdString(message->message_name()) << QString::fromStdString(message->sender()) << "->" << QString::fromStdString(message->receiver());
    }
    assert(!message->receiver().empty());

    auto id = _counter.fetch_add(1);
    message->_id = id;
    message->_time = mcc::misc::currentDateTime();
    _queue->send(std::move(message));
    return id;
}

MessageId MessageSenderX::sendTo(const std::string& to, MessagePtr&& message)
{
    message->_receiver = to;
    return send(std::move(message));
}

}
}

