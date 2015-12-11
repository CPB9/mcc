#include "photon/Reader.h"

#include <gtest/gtest.h>

class PhotonReaderTest : public ::testing::Test {
protected:
    template <std::size_t n, typename R>
    void init(const R (&array)[n])
    {
        PhotonReader_Init(&_reader, array, sizeof(R) * n);
    }

    void expectNextValue(PhotonBerValue expected)
    {
        PhotonBerValue value;
        PhotonResult rv = PhotonReader_ReadBer(&_reader, &value);
        ASSERT_EQ(rv, PhotonResult_Ok);
        ASSERT_EQ(expected, value);
    }

    void expectEndOfStream()
    {
        ASSERT_TRUE(PhotonReader_IsAtEnd(&_reader));
    }


private:
    PhotonReader _reader;
};

TEST_F(PhotonReaderTest, init)
{
}

TEST_F(PhotonReaderTest, readShortForm)
{
    uint8_t data[] = {0, 80, 127};
    init(data);
    expectNextValue(0);
    expectNextValue(80);
    expectNextValue(127);
    expectEndOfStream();
}

TEST_F(PhotonReaderTest, readLongForm)
{
    uint8_t data[] = {0x81, 0xff, 0x82, 0xaa, 0xbb, 0x83, 0xcc, 0xdd, 0xee,
                      0x88, 0x88, 0x77, 0x66, 0x55, 0x44, 0x33, 0x22, 0x11};
    init(data);
    expectNextValue(0xff);
    expectNextValue(0xbbaa);
    expectNextValue(0xeeddcc);
    expectNextValue(0x1122334455667788);
    expectEndOfStream();
}
