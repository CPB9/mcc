/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#pragma once
#include <vector>
#include <memory>
#include <string>
#include <set>

#include "mcc/messages/ServiceAbstract.h"

namespace mcc {
namespace core {
namespace manager {

class Service : public mcc::messages::ServiceAbstract
{
public:
    Service(const mcc::messages::LocalRouterPtr& router);
    virtual ~Service();
    static std::unique_ptr<mcc::messages::ServiceAbstract> startService(const std::string& name, const mcc::messages::LocalRouterPtr& router);

protected:
    bool pre() override;
    void post() override;

private:
    void process(std::unique_ptr<mcc::messages::SystemState_Request>&& message) override;
    void process(std::unique_ptr<mcc::messages::SystemComponentState>&& message) override;

private:
    bool isAllNeededStarted() const;

    std::string _routerAddress;
    std::set<std::string> _started;
    std::set<std::string> _needed;

    std::vector<std::unique_ptr<mcc::messages::ServiceAbstract>> _core;
    std::vector<std::unique_ptr<mcc::messages::ServiceAbstract>> _services;
};
}
}
}
