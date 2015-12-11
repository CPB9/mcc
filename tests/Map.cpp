#include "mcc/ui/map/MercatorProjection.h"
#include "mcc/ui/core/CoordinatePrinter.h"
#include "mcc/ui/map/FileCache.h"
#include "mcc/ui/map/drawables/PolyLine.h"
#include "mcc/ui/map/drawables/Point.h"

#include <gtest/gtest.h>

using namespace mcc::ui::map;

TEST(MercatorProjectionTest, sphericalMercator)
{
    MercatorProjection proj(MercatorProjection::SphericalMercator);
    EXPECT_FLOAT_EQ(4187591.89, proj.longitudeToX(37.617778));
    EXPECT_FLOAT_EQ(37.617778, proj.xToLongitude(4187591.89));
    EXPECT_FLOAT_EQ(7509137.58, proj.latitudeToY(55.751667));
    EXPECT_FLOAT_EQ(55.751667, proj.yToLatitude(7509137.58));
    EXPECT_DOUBLE_EQ(40075016.68557849, proj.equatorCircumference());
}

TEST(MercatorProjectionTest, worldMercator)
{
    MercatorProjection proj(MercatorProjection::EllipticalMercator);
    EXPECT_FLOAT_EQ(4187591.89, proj.longitudeToX(37.617778));
    EXPECT_FLOAT_EQ(37.617778, proj.xToLongitude(4187591.89));
    EXPECT_FLOAT_EQ(7473789.46, proj.latitudeToY(55.751667));
    EXPECT_FLOAT_EQ(55.751667, proj.yToLatitude(7473789.46));
    EXPECT_DOUBLE_EQ(40007862.917250335, proj.equatorCircumference());
}

static struct {
    TilePosition pos;
    QString str;
} posStr[] = {
    {TilePosition(1, 0, 0), "z1/0/x0/0/y0.jpg"},
    {TilePosition(14, 4776, 3376), "z14/4/x4776/3/y3376.jpg"},
    {TilePosition(20, 316729, 163421), "z20/309/x316729/159/y163421.jpg"},
};

TEST(FileCache, createPath)
{
    for (int i = 0; i < 1; i++) {
        auto str = FileCache::createPath(posStr[i].pos, "jpg");
        EXPECT_EQ(posStr[i].str, str);
    }
}

TEST(FileCache, createPosition)
{
    for (int i = 0; i < 1; i++) {
        auto pos = FileCache::createPosition(posStr[i].str, "jpg");
        EXPECT_TRUE(pos.isSome());
        if (pos.isSome()) {
            EXPECT_EQ(posStr[i].pos, pos.unwrap());
        }
    }
}

TEST(CoordinatePrinter, asd)
{
    using mcc::ui::core::degreeChar;
    mcc::ui::core::CoordinatePrinter printer;
    printer.setFormat(mcc::ui::core::CoordinateFormat::Degrees);

    EXPECT_DOUBLE_EQ(75.123, printer.parse(QString("75.123") + degreeChar).unwrap());
    EXPECT_DOUBLE_EQ(1, printer.parse(QString("1.") + degreeChar).unwrap());
    EXPECT_DOUBLE_EQ(0.123, printer.parse(QString(".123") + degreeChar).unwrap());

    printer.setFormat(mcc::ui::core::CoordinateFormat::DegreesMinutes);
}

TEST(PolyLineBase, hasPointInside)
{
    PolyLineBase<Point> polyline(MapRect::create(0));
    polyline.emplaceBack(QPointF(0, 0));
    polyline.emplaceBack(QPointF(10, 0));
    polyline.emplaceBack(QPointF(10, 10));
    polyline.emplaceBack(QPointF(0, 10));
    polyline.emplaceBack(QPointF(0, 0));
    EXPECT_TRUE(polyline.hasPointInside(QPointF(1, 1)));
    EXPECT_TRUE(polyline.hasPointInside(QPointF(1, 9)));
    EXPECT_TRUE(polyline.hasPointInside(QPointF(9, 1)));
    EXPECT_TRUE(polyline.hasPointInside(QPointF(9, 9)));
    EXPECT_TRUE(polyline.hasPointInside(QPointF(5, 5)));
    EXPECT_FALSE(polyline.hasPointInside(QPointF(-1, 5)));
    EXPECT_FALSE(polyline.hasPointInside(QPointF(5, 11)));
    EXPECT_FALSE(polyline.hasPointInside(QPointF(11, 5)));
    EXPECT_FALSE(polyline.hasPointInside(QPointF(5, -1)));
}

TEST(PolyLineBase, hasPointInsideSelfIntersecting)
{
    PolyLineBase<Point> polyline(MapRect::create(0));
    polyline.emplaceBack(QPointF(0, 0));
    polyline.emplaceBack(QPointF(0, 10));
    polyline.emplaceBack(QPointF(10, 0));
    polyline.emplaceBack(QPointF(10, 10));
    polyline.emplaceBack(QPointF(0, 0));
    EXPECT_TRUE(polyline.hasPointInside(QPointF(2, 6)));
    EXPECT_TRUE(polyline.hasPointInside(QPointF(7, 6)));
}
