/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include <QThread>
#include <QDebug>

#include "mcc/messages/ServiceAbstract.h"
#include "mcc/messages/MessageSender.h"
#include "mcc/messages/System.h"


namespace mcc {
namespace misc {

ServiceAbstract::~ServiceAbstract()
{
    assert(!isRunning_());
    ServiceAbstract::post();
}

bool ServiceAbstract::pre()
{
    if (_out)
    {
        _out->send<mcc::messages::SystemComponentStarted>();
        _out->send<mcc::messages::SystemStateRequest>();
    }
    return true;
}

void ServiceAbstract::tick()
{
    if (!_in)
    {
        assert(false);
        return;
    }
    auto m = _in->tryRecvFor(std::chrono::milliseconds(100));
    if (m.isErr())
        return;
    mcc::messages::MessageProcessor::chooseProcessor(m.take(), this);
}

void ServiceAbstract::post()
{
    if (_out)
        _out->send<mcc::messages::SystemComponentStoped>();
    if (_in)
        _in->close();
}

class StartWaiter : public mcc::messages::MessageProcessor
{
public:
    virtual ~StartWaiter(){}
    bool isStarted() const { return _isStarted; }
    void process(std::unique_ptr<mcc::messages::SystemStarted>&& msg) override
    {
        _isStarted = true;
        _msg.swap(msg);
    }
    std::unique_ptr<mcc::messages::SystemStarted>&& take() { return std::move(_msg); }
private:
    bool _isStarted = false;
    std::unique_ptr<mcc::messages::SystemStarted> _msg;
};

bool ServiceAbstract::waitSystemStarted_(std::size_t msTimeout)
{
    while (isRunning_())
    {
        auto r = _in->tryRecvFor(std::chrono::milliseconds(100));
        if (r.isErr())
            continue;

        auto m = r.take();
        StartWaiter waiter;
        mcc::messages::MessageProcessor::chooseProcessor(std::move(m), &waiter);
        if (!waiter.isStarted())
            continue;

        mcc::messages::MessageProcessor::chooseProcessor(waiter.take(), this);
        return true;
    }
    return false;
}

}
}
