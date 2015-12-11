#include "mcc/misc/Channel.h"

#include <gtest/gtest.h>

#include <thread>
#include <vector>

using namespace mcc::misc;

TEST(ChannelTest, sendRecvInt)
{
    Channel<int> channel;
    std::thread sender([&]() {
        for (int i = 0; i < 100; i++) {
            channel.send(i);
        }
    });
    std::thread reciever([&]() {
        for (int i = 0; i < 100; i++) {
            Option<int> option = channel.recv();
            EXPECT_TRUE(option.isSome());
            EXPECT_EQ(i, option.unwrap());
        }
    });
    sender.join();
    reciever.join();
}

TEST(ChannelTest, moveRecvString)
{
    Channel<std::string> channel;
    std::thread reciever([&]() {
        for (int i = 0; i < 100; i++) {
            Option<std::string> option = channel.recv();
            EXPECT_TRUE(option.isSome());
            std::string expected = "test" + std::to_string(i);
            EXPECT_EQ(expected, option.unwrap());
        }
    });
    std::thread sender([&]() {
        for (int i = 0; i < 100; i++) {
            std::string value = "test" + std::to_string(i);
            channel.send(std::move(value));
        }
    });
    reciever.join();
    sender.join();
}

TEST(ChannelTest, emplaceRecvPair)
{
    Channel<std::pair<int, std::string>> channel;
    std::thread reciever([&]() {
        for (int i = 0; i < 100; i++) {
            Option<std::pair<int, std::string>> option = channel.recv();
            EXPECT_TRUE(option.isSome());
            std::string expectedStr = "test" + std::to_string(i);
            EXPECT_EQ(i, option.unwrap().first);
            EXPECT_EQ(expectedStr, option.unwrap().second);
        }
    });
    std::thread sender([&]() {
        for (int i = 0; i < 100; i++) {
            std::string value = "test" + std::to_string(i);
            channel.sendEmplace(i, value);
        }
    });
    reciever.join();
    sender.join();
}

TEST(ChannelTest, close)
{
    Channel<int> channel;
    std::thread reciever([&]() {
        while (true) {
            Option<int> option = channel.recv();
            if (option.isNone()) {
                return;
            }
        }
    });
    std::thread sender([&]() {
        while (true) {
            if (!channel.send(1)) {
                return;
            }
        }
    });
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    channel.close();
    reciever.join();
    sender.join();
}

TEST(ChannelTest, tryRecvAsync)
{
    Channel<int> channel;
    std::atomic_bool isRunning;
    isRunning = true;
    int counter = 0;
    std::thread reciever([&]() {
        while (true) {
            Result<int, ChannelError> res = channel.tryRecv();
            if (res.isErr()) {
                if (res.unwrapErr() == ChannelError::Closed) {
                    return;
                }
            } else {
                counter++;
                if (counter > 100) {
                    isRunning = false;
                    return;
                }
            }
        }
    });
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    std::thread sender([&]() {
        while (true) {
            if (!channel.send(1)) {
                return;
            }
        }
    });
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    while (isRunning) {
    }
    channel.close();
    reciever.join();
    sender.join();
}

TEST(ChannelTest, tryRecvSync)
{
    Channel<int> channel;
    channel.send(1);
    channel.send(2);
    auto result1 = channel.tryRecv();
    EXPECT_TRUE(result1.isOk());
    EXPECT_EQ(result1.unwrap(), 1);
    auto result2 = channel.tryRecv();
    EXPECT_TRUE(result2.isOk());
    EXPECT_EQ(result2.unwrap(), 2);
    auto result3 = channel.tryRecv();
    EXPECT_TRUE(result3.isErr());
    EXPECT_EQ(result3.unwrapErr(), ChannelError::Empty);
    channel.close();
    auto result4 = channel.tryRecv();
    EXPECT_TRUE(result4.isErr());
    EXPECT_EQ(result4.unwrapErr(), ChannelError::Closed);
}

TEST(ChannelTest, tryRecvFor)
{
    Channel<int> channel;
    int counter = 0;
    std::thread reciever([&]() {
        Result<int, ChannelError> res = channel.tryRecvFor(std::chrono::seconds(10));
        EXPECT_TRUE(res.isOk());
        EXPECT_EQ(1, res.unwrap());
    });
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    channel.send(1);
    reciever.join();
}

TEST(ChannelTest, tryRecvForTimeout)
{
    Channel<int> channel;
    int counter = 0;
    std::thread reciever([&]() {
        Result<int, ChannelError> res = channel.tryRecvFor(std::chrono::microseconds(1));
        EXPECT_TRUE(res.isErr());
        EXPECT_EQ(ChannelError::Timeout, res.unwrapErr());
    });
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    channel.send(1);
    reciever.join();
}

TEST(ChannelTest, tryRecvForClosed)
{
    Channel<int> channel;
    int counter = 0;
    std::thread reciever([&]() {
        Result<int, ChannelError> res = channel.tryRecvFor(std::chrono::seconds(10));
        EXPECT_TRUE(res.isErr());
        EXPECT_EQ(ChannelError::Closed, res.unwrapErr());
    });
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    channel.close();
    reciever.join();
}

TEST(ChannelTest, clear)
{
    Channel<int> channel;
    for (int i = 0; i < 10; i++) {
        channel.send(i);
    }
    EXPECT_FALSE(channel.isEmpty());
    channel.clear();
    EXPECT_TRUE(channel.isEmpty());
}

// пример использвания

TEST(ChannelPairTest, basic)
{
    ChannelPair<std::string> pair = makeChannel<std::string>();

    Sender<std::string> sender1 = pair.sender;
    Sender<std::string> sender2 = pair.sender;
    Sender<std::string> sender3 = pair.sender;

    Reciever<std::string> reciever1 = pair.reciever;
    Reciever<std::string> reciever2 = pair.reciever;

    sender1.send("one");
    sender1.send("two");
    sender2.send("three");
    sender3.send("four");

    EXPECT_EQ("one", reciever2.recv().unwrap());
    EXPECT_EQ("two", reciever1.recv().unwrap());

    sender2.send("five");

    EXPECT_EQ("three", reciever2.recv().unwrap());
    EXPECT_EQ("four", reciever2.recv().unwrap());
    EXPECT_EQ("five", reciever1.recv().unwrap());
}

TEST(BiChannelPairTest, basic)
{
    BiChannelPair<int, std::string> pair = makeBiChannel<int, std::string>();

    ChannelPair<int, std::string> forwardPair = pair.forward;
    ChannelPair<std::string, int> backwardPair = pair.backward;

    Sender<int> leftSender =  forwardPair.sender;
    Reciever<std::string> leftReciever =  forwardPair.reciever;

    Sender<std::string> rightSender = backwardPair.sender;
    Reciever<int> rightReciever =  backwardPair.reciever;

    leftSender.send(1);
    leftSender.send(2);
    leftSender.send(3);
    rightSender.send("one");
    rightSender.send("two");
    rightSender.send("three");

    EXPECT_EQ(1, rightReciever.recv().unwrap());
    EXPECT_EQ(2, rightReciever.recv().unwrap());
    EXPECT_EQ(3, rightReciever.recv().unwrap());
    EXPECT_EQ("one", leftReciever.recv().unwrap());
    EXPECT_EQ("two", leftReciever.recv().unwrap());
    EXPECT_EQ("three", leftReciever.recv().unwrap());

}
