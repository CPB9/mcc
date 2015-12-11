/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "mcc/Names.h"
#include "mcc/core/manager/Service.h"
#include "mcc/core/db/Service.h"
#include "mcc/core/tm/Service.h"
#include "mcc/core/router/Service.h"
#include "mcc/core/cmd/Service.h"
#include "mcc/encoder/internal/Service.h"
#include "mcc/encoder/mavlink/Service.h"
#include "mcc/encoder/photon/Service.h"
#include "mcc/modeling/SimpleModel.h"
#include "mcc/messages/System.h"


namespace mcc {
namespace core {
namespace manager {

Service::Service(const mcc::messages::LocalRouterPtr& router) : mcc::messages::ServiceAbstract(mcc::Names::coreManager(), router)
{
    _needed.insert(mcc::Names::coreDb());
    _needed.insert(mcc::Names::coreCmd());
    _needed.insert(mcc::Names::coreTm());
}

Service::~Service()
{
    finish(true);
}

std::unique_ptr<mcc::messages::ServiceAbstract> Service::startService(const std::string& name, const mcc::messages::LocalRouterPtr& router)
{
    if (name == mcc::Names::coreRouter())   return mcc::misc::Runnable::startInThread<mcc::core::router::Service>(router);
    if (name == mcc::Names::coreDb())       return mcc::misc::Runnable::startInThread<mcc::core::db::Service>(router);
    if (name == mcc::Names::coreCmd())      return mcc::misc::Runnable::startInThread<mcc::core::cmd::Service>(router);
    if (name == mcc::Names::coreTm())       return mcc::misc::Runnable::startInThread<mcc::core::tm::Service>(router);

    if (name == mcc::Names::encoderInternal())      return mcc::misc::Runnable::startInThread<mcc::encoder::internal::Service>(router);
    if (name == mcc::Names::encoderMavlink())       return mcc::misc::Runnable::startInThread<mcc::encoder::mavlink::Service>(router);
    if (name == mcc::Names::encoderPhoton())        return mcc::misc::Runnable::startInThread<mcc::encoder::photon::Service>(router);

    if (name == mcc::Names::model1())    return mcc::misc::Runnable::startInThread<mcc::modeling::Model1>(router, mcc::DefaultModelControlPort + 1);
    if (name == mcc::Names::model2())    return mcc::misc::Runnable::startInThread<mcc::modeling::Model2>(router, mcc::DefaultModelControlPort + 2);
    if (name == mcc::Names::model3())    return mcc::misc::Runnable::startInThread<mcc::modeling::Model3>(router, mcc::DefaultModelControlPort + 3);
    if (name == mcc::Names::model4())    return mcc::misc::Runnable::startInThread<mcc::modeling::Model4>(router, mcc::DefaultModelControlPort + 4);

    return nullptr;
}

bool Service::pre()
{
    if (!ServiceAbstract::pre())
        return false;

//     auto clients = _router->locals();
//     for (const auto i : clients)
//     {
//         _services.push_back(startService(i, _router));
//     }

    _core.push_back(startService(mcc::Names::coreRouter(), _router));
    _core.push_back(startService(mcc::Names::coreTm(), _router));
    _core.push_back(startService(mcc::Names::coreDb(), _router));
    _core.push_back(startService(mcc::Names::coreCmd(), _router));

    _services.push_back(startService(mcc::Names::encoderInternal(), _router));
    _services.push_back(startService(mcc::Names::encoderMavlink(), _router));
    _services.push_back(startService(mcc::Names::encoderPhoton(), _router));

    _services.push_back(startService(mcc::Names::model1(), _router));
    _services.push_back(startService(mcc::Names::model2(), _router));
    _services.push_back(startService(mcc::Names::model3(), _router));

    return true;
}

// mcc::misc::Runnable* getService()
// {
//
// }

void Service::post()
{
//     for (const auto& i : _servicesX)
//     {
//         i->requestInterruption();
//         i->quit();
//         i->wait();
//     }

    //выключаем всё кроме ядра
    for (const auto& i : _services)
    {
        if (_needed.find(i->name()) == _needed.end())
            i->finish();
    }
    for (const auto& i : _services)
    {
        if (_needed.find(i->name()) == _needed.end())
            i->finish(true);
    }

    //выключаем ядро, кроме роутера и бд
    for (const auto& i : _core)
    {
        if (i->name() != mcc::Names::coreDb() && i->name() != mcc::Names::coreRouter())
            i->finish();
    }
    for (const auto& i : _core)
    {
        if (i->name() != mcc::Names::coreDb() && i->name() != mcc::Names::coreRouter())
            i->finish(true);
    }


    //выключаем роутер
    for (const auto& i : _core)
    {
        if (i->name() == mcc::Names::coreRouter())
            i->finish(true);
    }

    //выключаем бд
    for (const auto& i : _core)
    {
        if (i->name() == mcc::Names::coreDb())
            i->finish(true);
    }

    for (const auto& i : _services)
    {
        i->finish(true);
    }

    for (const auto& i : _core)
    {
        i->finish(true);
    }
}

bool Service::isAllNeededStarted() const
{
    for (const auto& i : _needed)
    {
        if (_started.find(i) == _started.end())
            return false;
    }
    return true;
}

void Service::process(std::unique_ptr<mcc::messages::SystemComponentState>&& state)
{
    if (state->isStarted())
    {
        _started.insert(state->sender());
        if (isAllNeededStarted() && (_needed.find(state->sender()) != _needed.end()))
        {
            _out->send<mcc::messages::SystemState>(true);
        }
    }
    else
    {
        _started.erase(state->sender());
        if (!isAllNeededStarted() && (_needed.find(state->sender()) != _needed.end()))
        {
            _out->send<mcc::messages::SystemState>(false, state->sender());
        }
    }
}

void Service::process(std::unique_ptr<mcc::messages::SystemState_Request>&& request)
{
    _out->send<mcc::messages::SystemState>(*request, isAllNeededStarted());
}

}
}
}
