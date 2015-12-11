/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#pragma once

#include <cstdint>
#include <string>

#include "mcc/modeling/FSM.h"

namespace mcc
{
namespace modeling
{
enum class FlyingDeviceMode : uint8_t
{
    Off, // всё выключено
    PayloadReady, // готовность полезной нагрузки
    EngineReady, // готовность двигателя
    TakingOff, // взлетаем
    GuidedFlying, // летим без маршрута
    RouteFlying, // летим маршруту
    Waiting, // находимся в режиме ожидания
    Returning, // летим домой (если отдельно не задан "дом", то летим в точку старта")
    Landing, // садимся
};

class Command
{
public:
    Command(const std::string & name) : _name(name) {}
    Command(const Command & o) : _name(o._name) {}
    bool operator<(const Command & o) const { return _name < o._name; }
    bool operator==(const Command & o) const { return _name == o._name; }
    const std::string& name() const { return _name; }
private:
    std::string _name;
};

#ifdef MCC_DEFINE_COMMAND
#error Macro must not be defined here!
#endif
#define MCC_DEFINE_COMMAND(name) static const Command & name() { static Command cmd("Navigation." #name); return cmd; }

class NavigationCommands
{
public:
    explicit NavigationCommands() = delete;
    MCC_DEFINE_COMMAND(start)
    MCC_DEFINE_COMMAND(startEngine)
    MCC_DEFINE_COMMAND(takeOff)
    MCC_DEFINE_COMMAND(guidedFlying)
    MCC_DEFINE_COMMAND(flyRoute)
    MCC_DEFINE_COMMAND(wait)
    static const Command & _return() { static Command cmd("Navigation.return"); return cmd; }
    MCC_DEFINE_COMMAND(land)
    MCC_DEFINE_COMMAND(endLanging) // Нет такой команды, это псевдокоманда, этот момент нужно ещё продумать
    MCC_DEFINE_COMMAND(cancelLanging)
    MCC_DEFINE_COMMAND(stopEngine)
    MCC_DEFINE_COMMAND(stop)
};

#undef MCC_DEFINE_COMMAND

typedef FiniteStateMachineTransition<FlyingDeviceMode, Command> FlyingDeviceModesFsmTransition;
typedef FiniteStateMachine<FlyingDeviceMode, Command> FlyingDeviceModesFsm;
}
}

namespace std
{
template<>
struct hash<mcc::modeling::FlyingDeviceMode>
{
    typedef mcc::modeling::FlyingDeviceMode argument_type;
    typedef size_t result_type;

    result_type operator()(argument_type const& s) const
    {
        return hash<uint8_t>()(static_cast<uint8_t>(s));
    }
};

template<>
struct hash<mcc::modeling::Command>
{
    typedef mcc::modeling::Command argument_type;
    typedef size_t result_type;

    result_type operator()(argument_type const& s) const
    {
        return hash<string>()(s.name());
    }
};

}