#include <iostream>

#include <QCoreApplication>
#include <QDebug>       

#include "bmcl/Logging.h"

#include "mcc/core/decode/Sqlite3RegistryProvider.h"
#include "mcc/core/decode/DecodeSourcesRegistryProvider.h"

#include <gtest/gtest.h>

using namespace mcc::decode;

/*
namespace test

alias f32 float:32
alias u8 uint:8

type f32_arr [f32, 4]
type u8_dyn_arr [u8, 3..8]

type u8_enum enum u8 (zero = 0, one = 1, ten = 10)

type sub_struct struct(
    u8 first,
    u8_enum second,
    f32_arr arr
)

type struct_test struct(
    u8 u8,
    f32 f32,
    u8_dyn_arr dyn_arr,
    f32_arr arr,
    int:64 int_64,
    [sub_struct, 2..3] sub_structs
)

component SubComp
{
    struct(
        u8 a,
        uint:16 b,
        [sub_struct, 2] sub_structs
    )
}

component TestComp : SubComp
{
    struct_test

    command cmdZero:0()
    command cmdOne:1(u8 arg0, f32_arr arg1, u8_dyn_arr arg2, struct_test arg3)

    message msgZero:0 status *.*
    message msgOne:1 event (
        u8,
        sub_structs[1].arr[0],
        SubComp.a,
        SubComp.sub_structs[0].arr[1]
    )
}
*/

class DecodeTest : public ::testing::Test
{
public:
    DecodeTest()
    {
        if (_registrySqlite3.get() == nullptr)
        {
            Sqlite3RegistryProvider registryProvider(TEST_DATABASE);
            _registrySqlite3.reset(registryProvider.provide().release());
        }
        if (_registrySources.get() == nullptr)
        {
            std::string decodeDir(TEST_DECODE_DIR);
            DecodeSourcesRegistryProvider registryProvider({decodeDir + "/main.decode", decodeDir + "/test.decode"});
            _registrySources.reset(registryProvider.provide().release());
        }
    }
protected:
    static std::unique_ptr<Registry> _registrySqlite3;
    static std::unique_ptr<Registry> _registrySources;
};

std::unique_ptr<Registry> DecodeTest::_registrySqlite3;
std::unique_ptr<Registry> DecodeTest::_registrySources;

class AssertReader
{
public:
    explicit AssertReader(MapVariant &parameters, MemReader &assertReader) : _parameters(parameters), _assertReader(assertReader) {}
    AssertReader & uint8(const char * name) { EXPECT_EQ(_parameters.getValue(name).toUint64(), _assertReader.readUint8()); return *this; }
    AssertReader & uint16(const char * name) { EXPECT_EQ(_parameters.getValue(name).toUint64(), _assertReader.readUint16Le()); return *this; }
    AssertReader & int64(const char * name) { EXPECT_EQ(_parameters.getValue(name).toInt64(), _assertReader.readInt64Le()); return *this; }
    AssertReader & float32(const char * name) { EXPECT_EQ(_parameters.getValue(name).toFloat32(), _assertReader.readFloat32Le()); return *this; }
private:
    MapVariant & _parameters;
    MemReader & _assertReader;
};

TEST_F(DecodeTest, readMessageTest)
{
    std::shared_ptr<Message> messageOption = _registrySqlite3->messageByFqn("test.TestComp.msgOne");
    EXPECT_TRUE(static_cast<bool>(messageOption));
    Message & message = *messageOption;
    EXPECT_EQ(message.id(), 1);
    EXPECT_TRUE(dynamic_cast<EventMessage*>(&message) != nullptr);
    uint8_t buf[] = {45, 0x1, 0x2, 0x3, 0x4, 23, 0x5, 0x6, 0x7, 0x8};
    EXPECT_EQ(message.parameters().size(), 4);
    MemReader reader(buf);
    ValueOrError valueOrError(message.deserialize(reader));
    EXPECT_TRUE(valueOrError.isValue());
    MapVariant &parameters = valueOrError.value()->asMap();
    MemReader memReader(buf);
    AssertReader assertReader(parameters, memReader);
    assertReader.uint8("u8").float32("sub_structs[1].arr[0]").uint8("SubComp.a").float32("SubComp.sub_structs[0].arr[1]");
}

TEST_F(DecodeTest, readMessageSourcesTest)
{
    std::shared_ptr<Message> messageOption = _registrySources->messageByFqn("test.TestComp.msgOne");
    /*EXPECT_TRUE(messageOption);
    Message &message(*messageOption);
    EXPECT_EQ(message.id(), 1);
    EXPECT_TRUE(dynamic_cast<EventMessage*>(&message) != nullptr);
    uint8_t buf[] = {45, 0x1, 0x2, 0x3, 0x4, 23, 0x5, 0x6, 0x7, 0x8};
    EXPECT_EQ(message.parameters().size(), 4);
    MemReader reader(buf);
    ValueOrError valueOrError(message.deserialize(reader));
    EXPECT_TRUE(valueOrError.isValue());
    MapVariant &parameters(valueOrError.value()->asMap());
    MemReader memReader(buf);
    AssertReader assertReader(parameters, memReader);
    assertReader.uint8("u8").float32("sub_structs[1].arr[0]").uint8("SubComp.a").float32("SubComp.sub_structs[0].arr[1]");*/
}

TEST_F(DecodeTest, readComplexMessageTest)
{
    std::shared_ptr<Message> messageOption = _registrySqlite3->messageByFqn("test.TestComp.msgZero");
    EXPECT_TRUE(static_cast<bool>(messageOption));
    Message &zeroMessage(*messageOption);
    EXPECT_EQ(zeroMessage.id(), 0);
    EXPECT_TRUE(dynamic_cast<StatusMessage*>(&zeroMessage) != nullptr);
    uint8_t buf2[] = {
        /* SubComp.a */ 17, /* SubComp.b */ 0x10, 0x20,
        /* SubComp.sub_structs[0] */ 8, 9, 0x11, 0x22, 0x33, 0x44, 0x12, 0x23, 0x34, 0x45, 0x13, 0x24, 0x35, 0x46, 0x14, 0x25, 0x36, 0x47,
        /* SubComp.sub_structs[1] */ 18, 19, 0x21, 0x22, 0x33, 0x44, 0x22, 0x23, 0x34, 0x45, 0x23, 0x24, 0x35, 0x46, 0x24, 0x25, 0x36, 0x47,
        /* u8 */ 55, /* f32 */ 0x9, 0x10, 0x11, 0xFF,
        /* dyn_arr */ 4, 20, 30, 40, 50,
        /* arr */ 0x11, 0x22, 0x33, 0x44, 0x12, 0x23, 0x34, 0x45, 0x13, 0x24, 0x35, 0x46, 0x14, 0x25, 0x36, 0x47,
        /* int_64 */ 0xF, 0xE, 0xD, 0xC, 0xB, 0xA, 0x9, 0x8,
        2,
        /* sub_structs[0] */ 8, 9, 0x11, 0x22, 0x33, 0x44, 0x12, 0x23, 0x34, 0x45, 0x13, 0x24, 0x35, 0x46, 0x14, 0x25, 0x36, 0x47,
        /* sub_structs[1] */ 18, 19, 0x21, 0x22, 0x33, 0x44, 0x22, 0x23, 0x34, 0x45, 0x23, 0x24, 0x35, 0x46, 0x24, 0x25, 0x36, 0x47,
    };
    MemReader reader2(buf2);
    ValueOrError valueOrError(zeroMessage.deserialize(reader2));
    EXPECT_TRUE(valueOrError.isValue());
    MapVariant & parameters = valueOrError.value()->asMap();
    MemReader memReader(buf2);
    AssertReader assertReader(parameters, memReader);
    assertReader
        .uint8("SubComp.a")
        .uint16("SubComp.b")
        .uint8("SubComp.sub_structs[0].first")
        .uint8("SubComp.sub_structs[0].second")
        .float32("SubComp.sub_structs[0].arr[0]")
        .float32("SubComp.sub_structs[0].arr[1]")
        .float32("SubComp.sub_structs[0].arr[2]")
        .float32("SubComp.sub_structs[0].arr[3]")
        .uint8("SubComp.sub_structs[1].first")
        .uint8("SubComp.sub_structs[1].second")
        .float32("SubComp.sub_structs[1].arr[0]")
        .float32("SubComp.sub_structs[1].arr[1]")
        .float32("SubComp.sub_structs[1].arr[2]")
        .float32("SubComp.sub_structs[1].arr[3]")
        .uint8("u8")
        .float32("f32")
        .uint8("dyn_arr.size")
        .uint8("dyn_arr[0]")
        .uint8("dyn_arr[1]")
        .uint8("dyn_arr[2]")
        .uint8("dyn_arr[3]")
        .float32("arr[0]")
        .float32("arr[1]")
        .float32("arr[2]")
        .float32("arr[3]")
        .int64("int_64")
        .uint8("sub_structs.size")
        .uint8("sub_structs.[0].first")
        .uint8("sub_structs.[0].second")
        .float32("sub_structs.[0].arr[0]")
        .float32("sub_structs.[0].arr[1]")
        .float32("sub_structs.[0].arr[2]")
        .float32("sub_structs.[0].arr[3]")
        .uint8("sub_structs.[1].first")
        .uint8("sub_structs.[1].second")
        .float32("sub_structs.[1].arr[0]")
        .float32("sub_structs.[1].arr[1]")
        .float32("sub_structs.[1].arr[2]")
        .float32("sub_structs.[1].arr[3]");
}

TEST_F(DecodeTest, writeCmdTest)
{
    Command & cmdOne = *_registrySqlite3->rootNamespaceByName("test")->componentByName("TestComp")->commandByName("cmdOne");
    // command cmdOne:1(u8 arg0, f32_arr arg1, u8_dyn_arr arg2, struct_test arg3)
    MapVariant args;
    args.with("arg0", 10)
        .with("arg1", MapVariant().with("0", 0.1).with("1", 0.2).with("2", 0.3).with("3", 0.4))
        .with("arg2", MapVariant().with("size", 3).with("0", 10).with("1", 20).with("2", 30))
        .with("arg3",
/*
type struct_test struct(
    u8 u8,
    f32 f32,
    u8_dyn_arr dyn_arr,
    f32_arr arr,
    int:64 int_64,
    [sub_struct, 2..3] sub_structs
)
*/
              MapVariant().with("u8", 122).with("f32", -1.1)
                .with("dyn_arr", MapVariant().with("size", 3).with("0", 2).with("1", 4).with("2", 8))
                .with("arr", MapVariant().with("0", 0.1).with("1", 0.01).with("2", 0.001).with("3", 0.0001))
                .with("int_64", -334)
/*
type sub_struct struct(
    u8 first,
    u8_enum second,
    f32_arr arr
)
*/
                .with("sub_structs", MapVariant().with("size", 2)
                    .with("0", MapVariant().with("first", 12).with("second", 1)
                        .with("arr", MapVariant().with("0", 0.1).with("1", 0.2).with("2", 0.3).with("3", 0.4)))
                    .with("1", MapVariant().with("first", 121).with("second", 2)
                        .with("arr", MapVariant().with("0", 0.11).with("1", 0.21).with("2", 0.31).with("3", 0.41)))));
    uint8_t buf[1000];
    MemWriter writer(buf);
    EXPECT_TRUE(cmdOne.serialize(args, writer).isNone());
    MemReader memReader(buf);
    AssertReader assertReader(args, memReader);
    assertReader
        .uint8("arg0")
        .float32("arg1[0]").float32("arg1[1]").float32("arg1[2]").float32("arg1[3]")
        .uint8("arg2.size").uint8("arg2[0]").uint8("arg2[1]").uint8("arg2[2]")
        .uint8("arg3.u8").float32("arg3.f32").uint8("arg3.dyn_arr.size").uint8("arg3.dyn_arr[0]").uint8("arg3.dyn_arr[1]").uint8("arg3.dyn_arr[2]")
        .float32("arg3.arr[0]").float32("arg3.arr[1]").float32("arg3.arr[2]").float32("arg3.arr[3]")
        .int64("arg3.int_64")
        .uint8("arg3.sub_structs.size")
        .uint8("arg3.sub_structs[0].first").uint8("arg3.sub_structs[0].second")
        .float32("arg3.sub_structs[0].arr[0]").float32("arg3.sub_structs[0].arr[1]").float32("arg3.sub_structs[0].arr[2]").float32("arg3.sub_structs[0].arr[3]")
        .uint8("arg3.sub_structs[1].first").uint8("arg3.sub_structs[1].second")
        .float32("arg3.sub_structs[1].arr[0]").float32("arg3.sub_structs[1].arr[1]").float32("arg3.sub_structs[1].arr[2]").float32("arg3.sub_structs[1].arr[3]");
    EXPECT_EQ(writer.current(), memReader.current());
}