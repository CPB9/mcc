/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#pragma once
#include <map>
#include <string>

#include "mcc/Names.h"
#include "mcc/Settings.h"
#include "mcc/misc/Net.h"
#include "mcc/messages/ServiceAbstract.h"


namespace mcc {
namespace core {
namespace router {

class Poller;

class Service : public mcc::messages::ServiceAbstract
{
public:
    Service(const mcc::messages::LocalRouterPtr& router);
    virtual ~Service();

protected:
    void tick() override;

private:
    void routeMessage_(std::unique_ptr<mcc::messages::Message>&&);
    void sendMulticast(const std::string& from, mcc::messages::MessagePtr&& message);

    std::chrono::steady_clock::time_point _start;
};
}
}
}
