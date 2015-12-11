#include "mcc/misc/SharedVar.h"

#include <gtest/gtest.h>

#include <deque>
#include <thread>

using namespace mcc::misc;

TEST(SharedVar, intSimple)
{
    SharedVar<int> sharedVar;

    {
        SharedVarWriterLock<int> lock(sharedVar);
        lock.value() = 123;
    }

    {
        SharedVarReaderLock<int> lock(sharedVar);
        EXPECT_EQ(123, lock.value());
    }
}

TEST(SharedVar, intThreaded)
{
    SharedVarPtr<int> sharedVar = makeSharedVar<int>();

    std::thread t1([sharedVar]() {
        SharedVarReader<int> reader(sharedVar);

        for (int i = 0; i < 1000; i++) {
            SharedVarReaderLock<int> lock = reader.lock();
            EXPECT_EQ(i, lock.value());
        }
    });

    std::thread t2([sharedVar]() {
        SharedVarWriter<int> writer(sharedVar);

        for (int i = 0; i < 1000; i++) {
            SharedVarWriterLock<int> lock = writer.lock();
            lock.value() = i;
        }
    });

    t1.join();
    t2.join();
}

TEST(SharedVar, dequeThreaded)
{
    SharedVarPtr<std::deque<int>> sharedVar = makeSharedVar<std::deque<int>>();
    int sum1 = 0;
    int sum2 = 0;

    std::thread r1([sharedVar, &sum1]() {
        SharedVarReader<std::deque<int>> reader(sharedVar);

        for (int i = 0; i < 500; i++) {
            SharedVarReaderLock<std::deque<int>> lock = reader.lock();
            sum1 += lock->front();
            lock->pop_front();
        }
    });

    std::thread r2([sharedVar, &sum2]() {
        SharedVarReader<std::deque<int>> reader(sharedVar);

        for (int i = 0; i < 500; i++) {
            SharedVarReaderLock<std::deque<int>> lock = reader.lock();
            sum2 += lock->front();
            lock->pop_front();
        }
    });

    std::thread w1([sharedVar]() {
        SharedVarWriter<std::deque<int>> writer(sharedVar);

        for (int i = 0; i < 1000; i += 2) {
            SharedVarWriterLock<std::deque<int>> lock = writer.lock();
            lock->push_back(i);
        }
    });

    std::thread w2([sharedVar]() {
        SharedVarWriter<std::deque<int>> writer(sharedVar);

        for (int i = 1; i < 1000; i += 2) {
            SharedVarWriterLock<std::deque<int>> lock = writer.lock();
            lock->push_back(i);
        }
    });

    r1.join();
    r2.join();
    w1.join();
    w2.join();
    EXPECT_EQ(499500, sum1 + sum2);
}
