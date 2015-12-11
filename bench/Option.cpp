#include "mcc/misc/Option.h"

#include <benchmark/benchmark.h>

#include <cstdint>
#include <cstring>

using mcc::misc::Option;

#define NOINLINE __attribute__((noinline))

template <std::size_t size>
struct NonTrivial {
    NOINLINE NonTrivial()
    {
        std::memset(a, 1, size);
    }

    NOINLINE NonTrivial(const NonTrivial& other)
    {
        std::memcpy(a, other.a, size);
    }

    NOINLINE NonTrivial(NonTrivial&& other)
    {
        std::memcpy(a, other.a, size);
    }

    NOINLINE ~NonTrivial()
    {
        std::memset(a, 2, size);
    }

    NonTrivial& NOINLINE operator=(const NonTrivial& other)
    {
        std::memcpy(a, other.a, size);
        return *this;
    }

    NonTrivial& NOINLINE operator=(NonTrivial&& other)
    {
        std::memcpy(a, other.a, size);
        return *this;
    }

    uint8_t a[size];
};

template <typename T>
static void NOINLINE use(const T& t)
{
    (void)t;
}

template <std::size_t size>
static NonTrivial<size> NOINLINE f1(bool* isOk)
{
    *isOk = true;
    return NonTrivial<size>();
}

template <std::size_t size>
static void returnNonTrivialWithFlag(benchmark::State& state)
{
    bool isOk;
    while (state.KeepRunning()) {
        NonTrivial<size> value = f1<size>(&isOk);
        use(value);
    }
}

template <std::size_t size>
static Option<NonTrivial<size>> NOINLINE f2()
{
    return NonTrivial<size>();
}

template <std::size_t size>
static void returnNonTrivialWithOption(benchmark::State& state)
{
    while (state.KeepRunning()) {
        Option<NonTrivial<size>> value = f2<size>();
        use(value);
    }
}

template <typename T>
static T NOINLINE f3(bool* isOk)
{
    *isOk = true;
    return T(5);
}

template <typename T>
static void returnWithFlag(benchmark::State& state)
{
    bool isOk;
    while (state.KeepRunning()) {
        T value = f3<T>(&isOk);
        use(value);
    }
}

template <typename T>
static Option<T> NOINLINE f4()
{
    return T(5);
}

template <typename T>
static void returnWithOption(benchmark::State& state)
{
    while (state.KeepRunning()) {
        Option<T> value = f4<T>();
        use(value.unwrap());
    }
}

template <std::size_t size>
struct Trivial {
    Trivial(uint8_t value)
    {
        a[0] = value;
    }

    uint8_t a[size];
};

BENCHMARK_TEMPLATE(returnWithFlag, uint8_t);
BENCHMARK_TEMPLATE(returnWithOption, uint8_t);
BENCHMARK_TEMPLATE(returnWithFlag, uint16_t);
BENCHMARK_TEMPLATE(returnWithOption, uint16_t);
BENCHMARK_TEMPLATE(returnWithFlag, uint32_t);
BENCHMARK_TEMPLATE(returnWithOption, uint32_t);
BENCHMARK_TEMPLATE(returnWithFlag, uint64_t);
BENCHMARK_TEMPLATE(returnWithOption, uint64_t);
BENCHMARK_TEMPLATE(returnWithFlag, Trivial<128>);
BENCHMARK_TEMPLATE(returnWithOption, Trivial<128>);
BENCHMARK_TEMPLATE(returnWithFlag, Trivial<256>);
BENCHMARK_TEMPLATE(returnWithOption, Trivial<256>);
BENCHMARK_TEMPLATE(returnWithFlag, Trivial<65536>);
BENCHMARK_TEMPLATE(returnWithOption, Trivial<65536>);

BENCHMARK_TEMPLATE(returnNonTrivialWithFlag, 1);
BENCHMARK_TEMPLATE(returnNonTrivialWithOption, 1);
BENCHMARK_TEMPLATE(returnNonTrivialWithFlag, 2);
BENCHMARK_TEMPLATE(returnNonTrivialWithOption, 2);
BENCHMARK_TEMPLATE(returnNonTrivialWithFlag, 4);
BENCHMARK_TEMPLATE(returnNonTrivialWithOption, 4);
BENCHMARK_TEMPLATE(returnNonTrivialWithFlag, 8);
BENCHMARK_TEMPLATE(returnNonTrivialWithOption, 8);
BENCHMARK_TEMPLATE(returnNonTrivialWithFlag, 16);
BENCHMARK_TEMPLATE(returnNonTrivialWithOption, 16);
BENCHMARK_TEMPLATE(returnNonTrivialWithFlag, 32);
BENCHMARK_TEMPLATE(returnNonTrivialWithOption, 32);
BENCHMARK_TEMPLATE(returnNonTrivialWithFlag, 64);
BENCHMARK_TEMPLATE(returnNonTrivialWithOption, 64);
BENCHMARK_TEMPLATE(returnNonTrivialWithFlag, 128);
BENCHMARK_TEMPLATE(returnNonTrivialWithOption, 128);
BENCHMARK_TEMPLATE(returnNonTrivialWithFlag, 256);
BENCHMARK_TEMPLATE(returnNonTrivialWithOption, 256);
BENCHMARK_TEMPLATE(returnNonTrivialWithFlag, 65536);
BENCHMARK_TEMPLATE(returnNonTrivialWithOption, 65536);

BENCHMARK_MAIN();
