/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#pragma once

#include <set>
#include <vector>
#include <functional>
#include <unordered_map>

/**
 * Finite State Machine
 */

namespace mcc
{
namespace modeling
{

template<typename M, typename A>
class FiniteStateMachineTransition
{
public:
    FiniteStateMachineTransition(M from, A action, M to, std::function<void()> apply = nullptr, std::function<bool()> constraints = nullptr)
    : _from(from), _action(action), _to(to), _apply(apply), _constraints(constraints){}
    ~FiniteStateMachineTransition() {}
    FiniteStateMachineTransition(const FiniteStateMachineTransition<M, A> & o)
    : FiniteStateMachineTransition(o._from, o._action, o._to, o._apply, o._constraints){}
    FiniteStateMachineTransition(FiniteStateMachineTransition<M, A> && o)
    : _from(o._from), _action(o._action), _to(o._to), _apply(o._apply), _constraints(o._constraints){}
    FiniteStateMachineTransition<M, A> & operator=(const FiniteStateMachineTransition<M, A> & o) {
        _from = o._from;
        _action = o._action;
        _to = o._to;
        _apply = o._apply;
        _constraints = o._constraints;
        return *this;
    }
    M from() const { return _from; }
    A action() const { return _action; }
    M to() const { return _to; }
    std::function<bool()> constraints() const { return _constraints; }
    std::function<void()> apply() const { return _apply; }
private:
    M _from;
    A _action;
    M _to;
    std::function<void()> _apply;
    std::function<bool()> _constraints;
};
    
template<typename M, typename A>
class FiniteStateMachine
{
public:
    typedef std::unordered_map<A, FiniteStateMachineTransition<M, A>> InnerMap;
    
    explicit FiniteStateMachine(M start, std::vector<FiniteStateMachineTransition<M, A>> && transitions) : _transitions(std::move(transitions)), _mode(start) {
        initTable();
    }
    FiniteStateMachine(const FiniteStateMachine<M, A> & o) : _modes(o._modes), _transitions(o._transitions), _mode(o._mode){}
    bool activate(const A & action) {
        const auto & innerMapIt(_table.find(_mode));
        if (innerMapIt == _table.end()) {
            return false;
        }
        InnerMap & inner(innerMapIt->second);
        const auto & transitionIt(inner.find(action));
        if (transitionIt == inner.end()) {
            return false;
        }
        FiniteStateMachineTransition<M, A> & transition(transitionIt->second);
        if (transition.constraints() == nullptr || transition.constraints()()) {
            if (transition.apply() != nullptr) {
                transition.apply()();
            }
            _mode = transition.to();
            return true;
        }
        return false;
    }
    
    bool isValidAction(const A & action) const {
        return _actions.find(action) != _actions.end();
    }
    
    const M & mode() const {
        return _mode;
    }

    void mode(M mode) { _mode = mode; }
    
private:
    std::set<M> _modes;
    std::vector<FiniteStateMachineTransition<M, A>> _transitions;
    M _mode;
    std::unordered_map<M, InnerMap> _table;
    std::set<A> _actions;
    
    void initTable()
    {
        _table.clear();
        _actions.clear();
        _modes.insert(_mode);
        for (const auto & transition : _transitions) {
            const M & from(transition.from());
            if (_table.find(from) == _table.end()) {
                _table.emplace(from, InnerMap());
            }
            InnerMap & inner(_table.at(from));
            const A & action(transition.action());
            BMCL_ASSERT(inner.find(action) == inner.end());
            _actions.insert(action);
            inner.emplace(action, transition);
            const M & to(transition.to());
            _modes.insert(from);
            _modes.insert(to);
        }
    }
};
    
}
}