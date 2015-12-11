/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "mcc/Names.h"
#include "mcc/messages/Tm.h"
#include "mcc/messages/MessageSender.h"
#include "mcc/core/tm/Service.h"


namespace mcc {
namespace core {
namespace tm {

Service::Service(const mcc::messages::LocalRouterPtr& router) : mcc::messages::ServiceAbstract(mcc::Names::coreTm(), router)
{
}

Service::~Service()
{
    finish(true);
}

void Service::process(std::unique_ptr<mcc::messages::TmParamSubscribe_Request>&& request)
{
    _paramSubscribers.insert(request->sender());
}

void Service::process(std::unique_ptr<mcc::messages::TmParamList>&& message)
{
    for (const auto& i : _paramSubscribers)
    {
        _out->sendTo(i, message->clone());
    }
    _out->sendTo(mcc::Names::coreDb(), std::move(message));
}

}
}
}
