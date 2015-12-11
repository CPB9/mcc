/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#pragma once

#include "mcc/ui/map/MercatorProjection.h"
#include "mcc/ui/map/Ptr.h"
#include "mcc/misc/Option.h"

#include <QSize>
#include <QPoint>
#include <QRect>
#include <QDialog>

#include <cmath>
#include <memory>

namespace mcc {
namespace ui {
namespace map {

class MapRect : public std::enable_shared_from_this<MapRect> {
public:
    MapRect(const MercatorProjection& proj, QWidget* parent);
    explicit MapRect(QWidget* parent);

    static MapRectPtr create(QWidget* parent);

    QPoint toWindowSystemCoordinates(const QPoint& mapOffset) const;
    QPoint fromWindowSystemCoordinates(const QPoint& pos) const;
    int zoomLevel() const;
    const QSize& size() const;
    const QPoint& offset() const;
    QRectF visibleMapRect() const;
    int maxMapSize() const;
    QPoint mapOffset(int x = 0, int y = 0) const;
    QPoint mapOffset(const QPoint& pos) const;
    const QPoint& mapOffsetRaw() const;
    QPoint mapOffsetFull(int x = 0, int y = 0) const;
    void execWidget(const QPointF& mapPos, QWidget* widget);
    int mapOffsetX(int x = 0) const;
    int mapOffsetY(int y = 0) const;
    double lon(int x = 0) const;
    double lat(int y = 0) const;
    core::LatLon latLon(const QPoint& pos) const;
    core::LatLon centerLatLon() const;
    const MercatorProjection& projection() const;
    bool isCenteredVertically() const;
    double relativeOffsetY(int y) const;
    double relativeOffsetX(int x) const;
    int fromRelativeOffsetY(double ry) const;
    int fromRelativeOffsetX(double rx) const;
    double horisontalPixelLength(int y) const;
    core::LatLon latLonOnLine(const core::LatLon& latLon, double angle, double mapOffset) const;

    void centerOn(const core::GeoBox& box);
    void centerOn(double lat, double lon);
    void centerOn(double lat, double lon, int zoomLevel);
    void setProjection(const MercatorProjection& proj);
    void setZoomLevel(int zoomLevel);
    void scroll(int dx, int dy);
    void zoom(const QPoint& p, int angle);
    void resize(int width, int height);
    void resize(const QSize& size);
    Qt::KeyboardModifiers modifiers() const;
    void setModifiers(Qt::KeyboardModifiers modifiers);
    void setCursorPosition(const QPoint& pos);
    void resetCursorPosition();
    const misc::Option<QPoint>& cursorPosition() const;

    double angleBetween(const core::LatLon& first, const core::LatLon& second) const;

    QWidget* parent() const;

private:
    void adjustMaxSize(int zoomLevel);
    void adjustMapOffsets();

    MercatorProjection _proj;
    QPoint _offset;
    QSize _size;
    misc::Option<QPoint> _cursorPos;
    int _maxMapSize;
    int _zoomLevel;
    bool _isCenteredVertically;
    QWidget* _parent;
    Qt::KeyboardModifiers _modifiers;
};

inline QWidget* MapRect::parent() const
{
    return _parent;
}

inline MapRectPtr MapRect::create(QWidget* parent)
{
    return std::make_shared<MapRect>(parent);
}

inline void MapRect::resize(int width, int height)
{
    resize(QSize(width, height));
}

inline void MapRect::setZoomLevel(int zoomLevel)
{
    int zoomDelta = zoomLevel - _zoomLevel;
    zoom(QPoint(_size.width() / 2, _size.height() / 2), zoomDelta);
}

inline QRectF MapRect::visibleMapRect() const
{
    return QRectF(mapOffset(), _size);
}

inline int MapRect::zoomLevel() const
{
    return _zoomLevel;
}

inline void MapRect::adjustMaxSize(int zoomLevel)
{
    _maxMapSize = std::exp2(zoomLevel - 1) * 256;
    _isCenteredVertically = _maxMapSize < _size.height();
}

inline double MapRect::relativeOffsetX(int x) const
{
    double offset = _offset.x() + x;
    return offset / double(_maxMapSize) * 2.0 - 1;
}

inline double MapRect::lat(int y) const
{
    double ry = relativeOffsetY(y);
    return _proj.relativeOffsetToLatitude(ry);
}

inline double MapRect::lon(int x) const
{
    double rx = relativeOffsetX(x);
    return _proj.relativeOffsetToLongitude(rx);
}

inline int MapRect::mapOffsetX(int x) const
{
    return (_offset.x() + x) % _maxMapSize;
}

inline QPoint MapRect::mapOffsetFull(int x, int y) const
{
    return QPoint(_offset.x() + x, mapOffsetY(y));
}

inline QPoint MapRect::mapOffset(int x, int y) const
{
    return QPoint(mapOffsetX(x), mapOffsetY(y));
}

inline QPoint MapRect::mapOffset(const QPoint& pos) const
{
    return QPoint(mapOffsetX(pos.x()), mapOffsetY(pos.y()));
}

inline bool MapRect::isCenteredVertically() const
{
    return _isCenteredVertically;
}

inline const QPoint& MapRect::offset() const
{
    return _offset;
}

inline const QSize& MapRect::size() const
{
    return _size;
}

inline int MapRect::maxMapSize() const
{
    return _maxMapSize;
}

inline const MercatorProjection& MapRect::projection() const
{
    return _proj;
}

inline core::LatLon MapRect::centerLatLon() const
{
    core::LatLon latLon;
    latLon.latitude = lat(size().height() / 2);
    latLon.longitude = lon(size().width() / 2);
    return latLon;
}

inline core::LatLon MapRect::latLon(const QPoint& pos) const
{
    core::LatLon latLon;
    latLon.latitude = lat(pos.y());
    latLon.longitude = lon(pos.x());
    return latLon;
}

inline const QPoint& MapRect::mapOffsetRaw() const
{
    return _offset;
}

inline void MapRect::setModifiers(Qt::KeyboardModifiers modifiers)
{
    _modifiers = modifiers;
}

inline Qt::KeyboardModifiers MapRect::modifiers() const
{
    return _modifiers;
}

inline const misc::Option<QPoint>& MapRect::cursorPosition() const
{
    return _cursorPos;
}

inline void MapRect::resetCursorPosition()
{
    _cursorPos = misc::None;
}

inline void MapRect::setCursorPosition(const QPoint& pos)
{
    _cursorPos = pos;
}
}
}
}
