/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#pragma once
#include <map>
#include <string>
#include <vector>
#include "mcc/messages/Deaclarations.h"


namespace mcc { namespace core { namespace router { class Service; } } }

namespace mcc {
namespace messages {

class LocalRouter
{
public:
    friend mcc::core::router::Service;
    LocalRouter();
    std::vector<std::string> locals() const;
    void add(const std::string& client);
    inline void lock() const { _isLocked = true; }
    mcc::messages::MessageQueue recv(const std::string& client) const;
    mcc::messages::MessageSender send(const std::string& client) const;

private:
    mutable bool _isLocked;
    mcc::messages::MessageQueue _in;
    std::map<std::string, mcc::messages::MessageQueue> _queueByName;
};
typedef std::shared_ptr<LocalRouter> LocalRouterPtr;

}
}
