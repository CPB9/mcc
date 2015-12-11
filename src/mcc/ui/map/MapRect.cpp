/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "mcc/ui/map/MapRect.h"
#include "mcc/ui/map/drawables/WithPosition.h"
#include "mcc/ui/map/WebMapProperties.h"

#include <QWidget>
#include <QDebug>

#include <cmath>

namespace mcc {
namespace ui {
namespace map {

MapRect::MapRect(const MercatorProjection& proj, QWidget* parent)
    : _proj(proj)
    , _offset(0, 0)
    , _size(0, 0)
    , _maxMapSize(256)
    , _zoomLevel(1)
    , _isCenteredVertically(false)
    , _parent(parent)
    , _modifiers(Qt::NoModifier)
{
}

MapRect::MapRect(QWidget* parent)
    : MapRect(MercatorProjection(MercatorProjection::SphericalMercator), parent)
{
}

QPoint MapRect::toWindowSystemCoordinates(const QPoint& mapOffset) const
{
    return _parent->mapToGlobal(mapOffset - this->mapOffsetFull());
}

QPoint MapRect::fromWindowSystemCoordinates(const QPoint& pos) const
{
    return _parent->mapFromGlobal(pos) + mapOffsetFull();
}

void MapRect::execWidget(const QPointF& mapPos, QWidget* widget)
{
    Q_UNUSED(mapPos);
    QPoint pos = QCursor::pos();
    widget->setParent(parent());
    widget->setWindowFlags(Qt::Window);
    widget->move(pos);
    widget->setWindowModality(Qt::ApplicationModal);
    widget->show();
}

void MapRect::resize(const QSize& size)
{
    _size = size;
    adjustMaxSize(_zoomLevel);
    adjustMapOffsets();
}

void MapRect::scroll(int dx, int dy)
{
    _offset.rx() -= dx;
    _offset.ry() -= dy;
    adjustMapOffsets();
}

void MapRect::zoom(const QPoint& p, int angle)
{
    int zoomLevel = _zoomLevel + angle;
    if (zoomLevel < 1) {
        return;
    }
    _maxMapSize = std::exp2(zoomLevel - 1) * 256;
    QPoint zoomPoint = _offset + p;
    zoomPoint *= std::exp2(angle);
    _offset = zoomPoint - p;
    _offset.rx() += _maxMapSize * (p.x() / _maxMapSize);
    adjustMaxSize(zoomLevel);
    adjustMapOffsets();
    _zoomLevel = zoomLevel;
    if (_cursorPos.isSome()) {
        _cursorPos.unwrap() *= std::exp2(angle);
    }
}

void MapRect::adjustMapOffsets()
{
    _offset.rx() += _maxMapSize;
    _offset.rx() %= _maxMapSize;
    if (_isCenteredVertically) {
        _offset.setY(-(_size.height() - _maxMapSize) / 2);
    } else {
        if (_offset.y() < 0) {
            _offset.setY(0);
        } else if (_offset.y() + _size.height() > _maxMapSize) {
            _offset.setY(_maxMapSize - _size.height());
        }
    }
}

double MapRect::relativeOffsetY(int y) const
{
    double relativeMapOffset;
    if (_isCenteredVertically) {
        relativeMapOffset = 0; // FIXME
    } else {
        relativeMapOffset = 0.5 - double(_offset.y() + y) / double(_maxMapSize);
    }
    return relativeMapOffset;
}

double MapRect::horisontalPixelLength(int y) const
{
    double yCoord = relativeOffsetY(y) * _proj.mapWidth();
    double maxWidthInPixels = _maxMapSize;
    return _proj.mapWidth() / maxWidthInPixels / _proj.scalingFactorFromY(yCoord);
}

int MapRect::fromRelativeOffsetX(double rx) const
{
    return (1 + rx) * (_maxMapSize) / 2 - _size.width() / 2;
}

int MapRect::fromRelativeOffsetY(double ry) const
{
    return (0.5 - ry) * (_maxMapSize)-_size.height() / 2;
}

void MapRect::centerOn(double lat, double lon)
{
    double rx = _proj.longitudeToRelativeOffset(lon);
    double ry = _proj.latitudeToRelativeOffset(lat);
    double x = (1 + rx) * (_maxMapSize) / 2 - _size.width() / 2;
    double y = (0.5 - ry) * (_maxMapSize)-_size.height() / 2;
    _offset.setX(x);
    _offset.rx() += _maxMapSize * (_size.width() / 2 / _maxMapSize);
    _offset.setY(y);
    adjustMapOffsets();
}

void MapRect::centerOn(double lat, double lon, int zoomLevel)
{
    _zoomLevel = zoomLevel;
    adjustMaxSize(zoomLevel);
    centerOn(lat, lon);
}

void MapRect::setProjection(const MercatorProjection& proj)
{
    double relativeMapOffset = relativeOffsetY(_size.height() / 2);
    double lat = _proj.relativeOffsetToLatitude(relativeMapOffset);
    _proj = proj;
    relativeMapOffset = _proj.latitudeToRelativeOffset(lat);
    relativeMapOffset = 0.5 - relativeMapOffset;
    _offset.ry() = relativeMapOffset * _maxMapSize - _size.height() / 2;
}

int MapRect::mapOffsetY(int y) const
{
    int py;
    if (_isCenteredVertically) {
        py = -(_size.height() - _maxMapSize) / 2 + y;
    } else {
        py = _offset.y() + y;
    }
    return py;
}

double MapRect::angleBetween(const core::LatLon& first, const core::LatLon& second) const
{
    MapRectConstPtr sharedThis = shared_from_this();
    QPointF a = WithPosition<>::fromLatLon(first, sharedThis);
    QPointF b = WithPosition<>::fromLatLon(second, sharedThis);
    return WithPosition<>::angleBetween(a, b);
}

core::LatLon MapRect::latLonOnLine(const core::LatLon& latLon, double angle, double mapOffset) const
{
    angle = qDegreesToRadians(angle);
    MapRectConstPtr sharedThis = shared_from_this();
    QPointF p = WithPosition<>::fromLatLon(latLon, sharedThis);
    QPointF next = p + QPointF(std::cos(angle) * mapOffset, -std::sin(angle) * mapOffset);
    return WithPosition<>::toLatLon(next, sharedThis);
}

void MapRect::centerOn(const core::GeoBox& box)
{
    MapRectConstPtr sharedThis = shared_from_this();
    QPointF topLeft = WithPosition<>::fromLatLon(box.topLeft, sharedThis);
    QPointF bottomRight = WithPosition<>::fromLatLon(box.bottomRight, sharedThis);
    double dy = std::abs(topLeft.y() - bottomRight.y());
    double dx = std::abs(topLeft.x() - bottomRight.x());
    double ratio = std::max(dx / _size.width(), dy / _size.height());
    core::LatLon center = box.center();
    int zoomDelta = std::ceil(std::log2(ratio));
    int newZoom = _zoomLevel - zoomDelta;
    newZoom = std::min<int>(newZoom, WebMapProperties::maxZoom());
    newZoom = std::max<int>(newZoom, WebMapProperties::minZoom());
    centerOn(center.latitude, center.longitude, newZoom);
}

}
}
}
