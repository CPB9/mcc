/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#pragma once

#include "bmcl/Result.h"

#include <cassert>
#include <utility>
#include <type_traits>

// clang-format off
#define MCC_IF_RESULT(value, expr)                                                                                     \
    if (auto value = mcc::misc::makeResultMacroHelper(expr)) {                                                                    \
        return value.unwrapErr();                                                                                      \
    } else
// clang-format on

#define MCC_SET_IF_RESULT(value, expr)                                                                                 \
    do {                                                                                                               \
        auto _mcc_temp_ = expr;                                                                                        \
        if (_mcc_temp_.isOk()) {                                                                                       \
            value = _mcc_temp_.take();                                                                                 \
        } else {                                                                                                       \
            return _mcc_temp_.unwrapErr();                                                                             \
        }                                                                                                              \
    } while (0);

#define get_or_exit_on_error(response, expression)                                                                     \
    ;                                                                                                                  \
    auto temp_##response = expression;                                                                                 \
    if (temp_##response.isErr())                                                                                       \
        return temp_##response.unwrapErr();                                                                            \
    auto response = temp_##response.unwrap();

namespace mcc {
namespace misc {

template <typename T, typename E>
class ResultMacroHelper;

using bmcl::Result;

template <typename T, typename E>
class ResultMacroHelper : public Result<T, E> {
public:
    ResultMacroHelper(Result<T, E>&& result);
    operator bool() const;
};

template <typename T, typename E>
ResultMacroHelper<T, E>::ResultMacroHelper(Result<T, E>&& result)
    : Result<T, E>::Result(std::move(result))
{
}

template <typename T, typename E>
inline ResultMacroHelper<T, E>::operator bool() const
{
    return Result<T, E>::isErr();
}

template <typename T, typename E>
inline ResultMacroHelper<T, E> makeResultMacroHelper(Result<T, E>&& result)
{
    return ResultMacroHelper<T, E>(std::move(result));
}
}
}
