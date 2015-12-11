/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#pragma once
#include <set>
#include <string>
#include <unordered_map>

#include "mcc/messages/ServiceAbstract.h"


namespace mcc {
namespace core {
namespace tm {

class Service : public mcc::messages::ServiceAbstract
{
public:
    Service(const mcc::messages::LocalRouterPtr& router);
    virtual ~Service();

private:
    void process(std::unique_ptr<mcc::messages::TmParamSubscribe_Request>&&);
    void process(std::unique_ptr<mcc::messages::TmParamList>&&);

private:
    std::unordered_map<std::string, std::string> _paramToClients;
    std::set<std::string> _paramSubscribers;
};
}
}
}