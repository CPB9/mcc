/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#pragma once
#include <map>
#include <set>
#include <string>
#include "mcc/messages/ServiceAbstract.h"
#include "mcc/misc/Cmd.h"


namespace mcc {
namespace core {
namespace cmd {

class Service : public mcc::messages::ServiceAbstract
{
public:
    Service(const mcc::messages::LocalRouterPtr& router);
    virtual ~Service();

private:
    void process(std::unique_ptr<mcc::messages::Cmd>&&) override;
    void process(std::unique_ptr<mcc::messages::CmdCancel>&&) override;
    void process(std::unique_ptr<mcc::messages::CmdState>&&) override;
    void process(std::unique_ptr<mcc::messages::CmdSubscribe_Request>&&) override;

    struct Command
    {
        Command(const std::string& from, const std::string& registered, const mcc::messages::Cmd& cmd);
        std::string         _from;
        std::string         _registered;
        mcc::misc::CmdState _state;
    };

    typedef std::map<mcc::misc::CmdCollationId, Command> Commands;


    struct Device
    {
        std::string _name;
        std::string _exchanger;
        Commands    _cmds;
    };

    const std::size_t _CmdsLimit = 100;
    typedef std::map<std::string, Device> Devices;
    Devices _devices;
};
}
}
}