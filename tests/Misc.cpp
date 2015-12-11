#include "mcc/misc/Option.h"
#include "mcc/misc/Result.h"
#include "mcc/misc/Crc.h"
#include "mcc/misc/Net.h"
#include "mcc/misc/Protocol.h"
#include "mcc/misc/TimeUtils.h"

#include <gtest/gtest.h>

#include <string>

#include <QDateTime>
#include <QDebug>

using namespace mcc::misc;

// option tests

TEST(OptionTest, createNone)
{
    Option<int> option;
    EXPECT_FALSE(option.isSome());
    EXPECT_TRUE(option.isNone());
}

static Option<double> noneTestFunc()
{
    return None;
}

TEST(OptionTest, createFromTag)
{
    Option<double> option = noneTestFunc();
    EXPECT_FALSE(option.isSome());
    EXPECT_TRUE(option.isNone());
}


TEST(OptionTest, createInt)
{
    Option<int> option(5);
    EXPECT_TRUE(option.isSome());
    EXPECT_FALSE(option.isNone());
    EXPECT_EQ(5, option.unwrap());
}

TEST(OptionTest, createStringInPlace)
{
    std::string src("test string");
    Option<std::string> option(InPlace, src, 1, 9);
    EXPECT_TRUE(option.isSome());
    EXPECT_EQ("est strin", option.unwrap());
}


TEST(OptionTest, createCopyString)
{
    std::string test = "copy string test";
    Option<std::string> option(test);
    EXPECT_TRUE(option.isSome());
    EXPECT_FALSE(option.isNone());
    EXPECT_EQ("copy string test", option.unwrap());
}

TEST(OptionTest, createMoveString)
{
    std::string test = "move string test";
    Option<std::string> option(std::move(test));
    EXPECT_TRUE(option.isSome());
    EXPECT_FALSE(option.isNone());
    EXPECT_EQ("move string test", option.unwrap());
}

TEST(OptionTest, unwrapMutable)
{
    Option<std::string> option("string test");
    option.unwrap() = "123";
    EXPECT_TRUE(option.isSome());
    EXPECT_EQ("123", option.unwrap());
}

TEST(OptionTest, unwrapOrLvalue)
{
    Option<std::string> option1;
    Option<std::string> option2("test");
    EXPECT_EQ("result", option1.unwrapOr("result"));
    EXPECT_EQ("test", option2.unwrapOr("result"));
}

TEST(OptionTest, unwrapOrRvalue)
{
    EXPECT_EQ("result", Option<std::string>().unwrapOr("result"));
    EXPECT_EQ("test", Option<std::string>("test").unwrapOr("result"));
}

TEST(OptionTest, takeString)
{
    Option<std::string> option("take string test");
    std::string newString = option.take();
    EXPECT_FALSE(option.isSome());
    EXPECT_TRUE(option.isNone());
    EXPECT_EQ("take string test", newString);
}

TEST(OptionTest, copyConstructor)
{
    Option<std::string> oldOption("copy constructor test");
    Option<std::string> newOption(oldOption);
    EXPECT_TRUE(oldOption.isSome());
    EXPECT_FALSE(oldOption.isNone());
    EXPECT_EQ("copy constructor test", oldOption.unwrap());
    EXPECT_TRUE(newOption.isSome());
    EXPECT_FALSE(newOption.isNone());
    EXPECT_EQ("copy constructor test", newOption.unwrap());
}

TEST(OptionTest, moveConstructor)
{
    Option<std::string> oldOption("move constructor test");
    Option<std::string> newOption(std::move(oldOption));
    EXPECT_FALSE(oldOption.isSome());
    EXPECT_TRUE(oldOption.isNone());
    EXPECT_TRUE(newOption.isSome());
    EXPECT_FALSE(newOption.isNone());
    EXPECT_EQ("move constructor test", newOption.unwrap());
}

TEST(OptionTest, operatorEqNoneNone)
{
    Option<std::string> oldOption;
    Option<std::string> newOption;
    newOption = oldOption;
    EXPECT_TRUE(oldOption.isNone());
    EXPECT_TRUE(newOption.isNone());
}

TEST(OptionTest, operatorEqNoneSome)
{
    Option<std::string> oldOption;
    Option<std::string> newOption("some");
    newOption = oldOption;
    EXPECT_TRUE(oldOption.isNone());
    EXPECT_TRUE(newOption.isNone());
}

TEST(OptionTest, operatorEqSomeNone)
{
    Option<std::string> oldOption("some");
    Option<std::string> newOption;
    newOption = oldOption;
    EXPECT_TRUE(oldOption.isSome());
    EXPECT_TRUE(newOption.isSome());
    EXPECT_EQ("some", oldOption.unwrap());
    EXPECT_EQ("some", newOption.unwrap());
}

TEST(OptionTest, operatorEqSomeSome)
{
    Option<std::string> oldOption("some1");
    Option<std::string> newOption("some2");
    newOption = oldOption;
    EXPECT_TRUE(oldOption.isSome());
    EXPECT_TRUE(newOption.isSome());
    EXPECT_EQ("some1", oldOption.unwrap());
    EXPECT_EQ("some1", newOption.unwrap());
}

TEST(OptionTest, operatorEqNoneValue)
{
    Option<std::string> option;
    std::string value("test value");
    option = value;
    EXPECT_TRUE(option.isSome());
    EXPECT_EQ("test value", option.unwrap());
}

TEST(OptionTest, operatorEqSomeValue)
{
    Option<std::string> option("previous");
    std::string value("next");
    option = value;
    EXPECT_TRUE(option.isSome());
    EXPECT_EQ("next", option.unwrap());
}

TEST(OptionTest, operatorEqMoveNoneNone)
{
    Option<std::string> oldOption;
    Option<std::string> newOption;
    newOption = std::move(oldOption);
    EXPECT_TRUE(oldOption.isNone());
    EXPECT_TRUE(newOption.isNone());
}

TEST(OptionTest, operatorEqMoveNoneSome)
{
    Option<std::string> oldOption;
    Option<std::string> newOption("some");
    newOption = std::move(oldOption);
    EXPECT_TRUE(oldOption.isNone());
    EXPECT_TRUE(newOption.isNone());
}

TEST(OptionTest, operatorEqMoveSomeNone)
{
    Option<std::string> oldOption("some");
    Option<std::string> newOption;
    newOption = std::move(oldOption);
    EXPECT_TRUE(oldOption.isNone());
    EXPECT_TRUE(newOption.isSome());
    EXPECT_EQ("some", newOption.unwrap());
}

TEST(OptionTest, operatorEqMoveSomeSome)
{
    Option<std::string> oldOption("some1");
    Option<std::string> newOption("some2");
    newOption = std::move(oldOption);
    EXPECT_TRUE(oldOption.isNone());
    EXPECT_TRUE(newOption.isSome());
    EXPECT_EQ("some1", newOption.unwrap());
}

TEST(OptionTest, operatorEqMoveNoneValue)
{
    Option<std::string> option;
    std::string value("test value");
    option = std::move(value);
    EXPECT_TRUE(option.isSome());
    EXPECT_EQ("test value", option.unwrap());
}

TEST(OptionTest, operatorEqMoveSomeValue)
{
    Option<std::string> option("previous");
    std::string value("next");
    option = std::move(value);
    EXPECT_TRUE(option.isSome());
    EXPECT_EQ("next", option.unwrap());
}

TEST(OptionTest, operatorBool)
{
    Option<std::string> none;
    Option<std::string> some("some");
    EXPECT_TRUE(some.isSome());
    EXPECT_FALSE(none.isSome());
}

TEST(OptionTest, operatorEqEq)
{
    Option<std::string> none1;
    Option<std::string> none2;
    Option<std::string> some1("some");
    Option<std::string> some2("some");
    Option<std::string> some3("emos");
    EXPECT_TRUE(none1 == none2);
    EXPECT_TRUE(none2 == none1);
    EXPECT_TRUE(some1 == some2);
    EXPECT_TRUE(some2 == some1);
    EXPECT_FALSE(none1 == some1);
    EXPECT_FALSE(some2 == none2);
    EXPECT_FALSE(some1 == some3);
    EXPECT_FALSE(some3 == some2);
}

TEST(OptionTest, operatorDeref)
{
    Option<std::string> option("some");
    EXPECT_TRUE(option.isSome());
    EXPECT_STREQ("some", option->c_str());
    option->append(" test");
    EXPECT_STREQ("some test", option->c_str());
}

TEST(OptionTest, operatorInd)
{
    Option<std::string> option("some");
    EXPECT_TRUE(option.isSome());
    EXPECT_STREQ("some", (*option).c_str());
    (*option).append(" test");
    EXPECT_STREQ("some test", (*option).c_str());
}

// result tests

TEST(ResultTest, createCopyErr)
{
    std::string errString = "error";
    Result<int, std::string> result(errString);
    EXPECT_FALSE(result.isOk());
    EXPECT_TRUE(result.isErr());
    EXPECT_EQ("error", result.unwrapErr());
}

TEST(ResultTest, createMoveErr)
{
    std::string errString = "error";
    Result<int, std::string> result(std::move(errString));
    EXPECT_FALSE(result.isOk());
    EXPECT_TRUE(result.isErr());
    EXPECT_EQ("error", result.unwrapErr());
}

TEST(ResultTest, createCopyResult)
{
    std::vector<int> data({1, 2, 3, 4, 6});
    Result<std::vector<int>, std::string> result(data);
    EXPECT_TRUE(result.isOk());
    EXPECT_FALSE(result.isErr());
    EXPECT_EQ(std::vector<int>({1, 2, 3, 4, 6}), result.unwrap());
}

TEST(ResultTest, createMoveResult)
{
    std::vector<int> data({1, 2, 3, 4, 5});
    Result<std::vector<int>, std::string> result(std::move(data));
    EXPECT_TRUE(result.isOk());
    EXPECT_FALSE(result.isErr());
    EXPECT_EQ(std::vector<int>({1, 2, 3, 4, 5}), result.unwrap());
}

TEST(ResultTest, copyConstructErr)
{
    std::vector<int> err{3, 2, 1};
    Result<std::string, std::vector<int>> oldResult(err);
    Result<std::string, std::vector<int>> newResult(oldResult);
    EXPECT_FALSE(newResult.isOk());
    EXPECT_TRUE(newResult.isErr());
    EXPECT_EQ(std::vector<int>({3, 2, 1}), newResult.unwrapErr());
}

TEST(ResultTest, copyConstructResult)
{
    Result<std::string, int> oldResult("result");
    Result<std::string, int> newResult(oldResult);
    EXPECT_TRUE(newResult.isOk());
    EXPECT_FALSE(newResult.isErr());
    EXPECT_EQ("result", newResult.unwrap());
}

TEST(ResultTest, moveConstructErr)
{
    std::vector<int> err{3, 2, 1};
    Result<std::string, std::vector<int>> oldResult(err);
    Result<std::string, std::vector<int>> newResult(std::move(oldResult));
    EXPECT_FALSE(newResult.isOk());
    EXPECT_TRUE(newResult.isErr());
    EXPECT_EQ(std::vector<int>({3, 2, 1}), newResult.unwrapErr());
}

TEST(ResultTest, moveConstructResult)
{
    Result<std::string, int> oldResult("result");
    Result<std::string, int> newResult(std::move(oldResult));
    EXPECT_TRUE(newResult.isOk());
    EXPECT_FALSE(newResult.isErr());
    EXPECT_EQ("result", newResult.unwrap());
}

// crc

TEST(Crc, 123456789)
{
    EXPECT_EQ(0x4b37, crc16Modbus("123456789", 9));
    EXPECT_EQ(0x906e, crc16X25("123456789", 9));
    EXPECT_EQ(0x6f91, crc16Mcrf4xx("123456789", 9));
    EXPECT_EQ(0xcbf43926, crc32("123456789", 9));
}



TEST(DateTime, serializeDeserialize)
{
    QDateTime serializeTime(QDate(2015, 4, 6), QTime(17, 16, 12, 126));

    QString patternString = "2015-04-06T17:16:12.126";

    EXPECT_TRUE(patternString == serializeTime.toString(mcc::misc::dateSerializeFormat()));

    QDateTime deserializeTime = QDateTime::fromString(patternString, mcc::misc::dateSerializeFormat());
    EXPECT_TRUE(deserializeTime.date().year() == 2015);
    EXPECT_TRUE(deserializeTime.date().month() == 4);
    EXPECT_TRUE(deserializeTime.date().day() == 6);
    EXPECT_TRUE(deserializeTime.time().hour() == 17);
    EXPECT_TRUE(deserializeTime.time().minute() == 16);
    EXPECT_TRUE(deserializeTime.time().second() == 12);
    EXPECT_TRUE(deserializeTime.time().msec() == 126);
}


