/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#pragma once
#include <memory>
#include <string>
#include "mcc/misc/Net.h"
#include "mcc/misc/Runnable.h"
#include "mcc/messages/Deaclarations.h"
#include "mcc/messages/Message.h"
#include "mcc/messages/LocalRouter.h"


namespace mcc {
namespace messages {

class ServiceAbstract : public mcc::misc::Runnable, public mcc::messages::MessageProcessor
{
public:
    ServiceAbstract(const std::string& name, const LocalRouterPtr& router)
        : mcc::misc::Runnable(name)
        , mcc::messages::MessageProcessor(router->send(name))
        , _router(router)
        , _in(router->recv(name))
        , _out(router->send(name))
    {
    }
    virtual ~ServiceAbstract();

protected:
    bool pre() override;
    void tick() override;
    void post() override;
    bool waitSystemStarted_(std::size_t msTimeout = -1);

    mcc::messages::MessageQueue _in;
    mcc::messages::MessageSender _out;
    mcc::messages::LocalRouterPtr _router;

private:
    void run();
};
}
}