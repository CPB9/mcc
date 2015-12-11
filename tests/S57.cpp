#include "mcc/s57/Parser.h"

#include "bmcl/MemReader.h"

#include <QByteArray>
#include <QFile>
#include <QTime>
#include <QDebug>

#include <gtest/gtest.h>

#include <memory>
#include <cassert>

using namespace mcc::s57;

class S57Test : public ::testing::Test {
protected:
    QByteArray readFile()
    {
        QFile file("big_map.000");
        EXPECT_TRUE(file.open(QFile::ReadOnly));
        return file.readAll();
    }
};

#define CHECK(result)                                                                                                  \
    do {                                                                                                               \
        if (result.isErr()) {                                                                                          \
            qDebug() << "error:" << int(result.unwrapErr());                                                           \
            return;                                                                                                    \
        }                                                                                                              \
    } while (0);

TEST_F(S57Test, leader)
{
    QTime start;
    start.start();
    QByteArray data = readFile();
    bmcl::MemReader memReader(data.constData(), data.size());
    Parser parser;
    ParseResult<BaseCellFilePtr> rv = parser.parseBaseCellFile(&memReader);
    EXPECT_TRUE(rv.isOk());
    if (rv.isErr()) {
        qDebug() << "error:" << int(rv.unwrapErr());
        return;
    }
    qDebug() << data.size() << parser.history().size() <<  double(parser.history().size()) / double(data.size()) * 100;
    qDebug() << start.elapsed() << "msecs";
}
