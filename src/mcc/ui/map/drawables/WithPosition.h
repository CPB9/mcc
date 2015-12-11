/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#pragma once

#include "mcc/ui/map/MapRect.h"
#include "mcc/ui/core/GlobalCoordinatePrinter.h"
#include "mcc/ui/core/CoordinatePrinter.h"

#include <QApplication>
#include <QClipboard>

#include <QString>
#include <QDebug>

#include <type_traits>

namespace mcc {
namespace ui {
namespace core {
class CoordinatePrinter;
}
namespace map {

class MercatorProjection;

template <typename B = void>
class WithPosition {
public:
    const QPointF& position();
    void setPosition(const QPointF& position) const;
    QString printCoordinates(const MapRectConstPtr& rect, const core::CoordinatePrinter& printer);
    QString printCoordinates(const MapRectConstPtr& rect);
    void printCoordinatesToClipboard(const MapRectConstPtr& rect);

    void setLatLon(const core::LatLon& latLon, const MapRectConstPtr& rect);
    core::LatLon toLatLon(const MapRectConstPtr& rect) const;

    void changeProjection(const MapRectConstPtr& rect, const MercatorProjection& from, const MercatorProjection& to);

    static QPointF fromLatLon(const core::LatLon& latLon, const MapRectConstPtr& rect);
    static QPointF fromLatLon(const core::LatLon& latLon, const MapRectConstPtr& rect, const MercatorProjection& proj);
    static core::LatLon toLatLon(const QPointF& position, const MapRectConstPtr& rect);
    static core::LatLon toLatLon(const QPointF& position, const MapRectConstPtr& rect, const MercatorProjection& proj);
    static double angleBetween(const QPointF& first, const QPointF& second);
    static double angleBetween(const QPointF& p1, const QPointF& c, const QPointF& p2);

    static QPointF edgePoint(const QSizeF& size, Qt::Alignment alignment = Qt::AlignCenter);
};

template <typename B>
inline const QPointF& WithPosition<B>::position()
{
    return static_cast<B*>(this)->position();
}

template <typename B>
inline void WithPosition<B>::setPosition(const QPointF& position) const
{
    static_cast<B*>(this)->setPosition(position);
}

template <typename B>
inline void WithPosition<B>::setLatLon(const core::LatLon& latLon, const mcc::ui::map::MapRectConstPtr& rect)
{
    static_cast<B*>(this)->setPosition(fromLatLon(latLon, rect));
}

template <typename B>
inline core::LatLon WithPosition<B>::toLatLon(const mcc::ui::map::MapRectConstPtr& rect) const
{
    return toLatLon(static_cast<const B*>(this)->position(), rect);
}

template <typename B>
inline QPointF WithPosition<B>::fromLatLon(const core::LatLon& latLon, const mcc::ui::map::MapRectConstPtr& rect)
{
    return fromLatLon(latLon, rect, rect->projection());
}

template <typename B>
inline core::LatLon WithPosition<B>::toLatLon(const QPointF& position, const mcc::ui::map::MapRectConstPtr& rect)
{
    return toLatLon(position, rect, rect->projection());
}

template <typename B>
QString WithPosition<B>::printCoordinates(const MapRectConstPtr& rect, const core::CoordinatePrinter& printer)
{
    core::LatLon latLon = toLatLon(rect);
    QString lat = printer.printLat(latLon.latitude, latLon.longitude);
    QString lon = printer.printLon(latLon.latitude, latLon.longitude);
    return lat + ", " + lon;
}

template <typename B>
inline QString WithPosition<B>::printCoordinates(const MapRectConstPtr& rect)
{
    return printCoordinates(rect, core::GlobalCoordinatePrinter::instance()->printer());
}

template <typename B>
inline void WithPosition<B>::printCoordinatesToClipboard(const MapRectConstPtr& rect)
{
    QString text = printCoordinates(rect);
    QApplication::clipboard()->setText(text);
}

template <typename B>
inline void WithPosition<B>::changeProjection(const MapRectConstPtr& rect, const MercatorProjection& from,
                                              const MercatorProjection& to)
{
    core::LatLon latLon = toLatLon(static_cast<B*>(this)->position(), rect, from);
    static_cast<B*>(this)->setPosition(fromLatLon(latLon, rect, to));
}

template <typename B>
QPointF WithPosition<B>::fromLatLon(const core::LatLon& latLon, const MapRectConstPtr& rect,
                                    const MercatorProjection& proj)
{
    double rx = proj.longitudeToRelativeOffset(latLon.longitude);
    double ry = proj.latitudeToRelativeOffset(latLon.latitude);
    int maxMapSize = rect->maxMapSize();
    double x = (1 + rx) * maxMapSize / 2;
    double y = (0.5 - ry) * maxMapSize;
    return QPointF(x, y);
}

template <typename B>
core::LatLon WithPosition<B>::toLatLon(const QPointF& position, const MapRectConstPtr& rect,
                                       const MercatorProjection& proj)
{
    int maxMapSize = rect->maxMapSize();
    double rx = position.x() / maxMapSize * 2.0 - 1;
    double ry = 0.5 - position.y() / maxMapSize;
    double lat = proj.relativeOffsetToLatitude(ry);
    double lon = proj.relativeOffsetToLongitude(rx);
    return core::LatLon(lat, lon);
}

// ось y направлена вниз
template <typename B>
double WithPosition<B>::angleBetween(const QPointF& first, const QPointF& second)
{
    return qRadiansToDegrees(std::atan2(first.y() - second.y(), second.x() - first.x()));
}

template <typename B>
double WithPosition<B>::angleBetween(const QPointF& p1, const QPointF& c, const QPointF& p2)
{
    QPointF v1 = p1 - c;
    QPointF v2 = p2 - c;
    double dp = v1.x() * v2.x() + v1.y() * v2.y();
    double l1 = std::hypot(v1.x(), v1.y());
    double l2 = std::hypot(v2.x(), v2.y());
    qDebug() << dp / (l1 * l2);
    return qRadiansToDegrees(std::acos(dp / (l1 * l2)));
}

template <typename B>
QPointF WithPosition<B>::edgePoint(const QSizeF& size, Qt::Alignment alignment)
{
    QPointF p;
    if (alignment & Qt::AlignLeft) {
        p.setX(0);
    } else if (alignment & Qt::AlignRight) {
        p.setX(size.width());
    } else {
        p.setX(size.width() / 2);
    }
    if (alignment & Qt::AlignTop) {
        p.setY(0);
    } else if (alignment & Qt::AlignBottom) {
        p.setY(size.height());
    } else {
        p.setY(size.height() / 2);
    }
    return -p;
}
}
}
}
