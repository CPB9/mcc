/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "mcc/ui/map/layers/KmlPolyLineLayer.h"
#include "mcc/ui/map/KmlUtils.h"

#include <QPainter>
#include <QDebug>
#include <QMenu>
#include <QAction>

namespace mcc {
namespace ui {
namespace map {

static QPixmap createPixmap(bool isActive)
{
    QImage img(7, 7, QImage::Format_ARGB32_Premultiplied);
    if (isActive) {
        img.fill(Qt::black);
    } else {
        img.fill(Qt::white);
    }
    return QPixmap::fromImage(std::move(img));
}

BiMarker KmlPolyLineLayer::createMarker(const QPointF& pos)
{
    BiMarker m(pos);
    m.setActivePixmap(createPixmap(true));
    m.setInactivePixmap(createPixmap(false));
    return m;
}

KmlPolyLineLayer::KmlPolyLineLayer(const QString& label, const kmldom::CoordinatesPtr& coordinates,
                                   const kmldom::StylePtr& style, const MapRectConstPtr& rect)
    : KmlElementLayer(coordinates, style, rect)
    , _polyline(rect)
    , _isActive(false)
{
    (void)label;
    _activePixmap = createPixmap(true);
    qDebug() << coordinates->get_coordinates_array_size();
    for (std::size_t i = 0; i < coordinates->get_coordinates_array_size(); i++) {
        const kmlbase::Vec3& vec3 = coordinates->get_coordinates_array_at(i);
        core::LatLon latLon(vec3.get_latitude(), vec3.get_longitude());
        QPointF pos = WithPosition<>::fromLatLon(latLon, mapRect());
        _polyline.insert(i, createMarker(pos));
    }
    double lineWidth = 1;
    QColor lineColor = Qt::white;
    if (style) {
        if (style->has_linestyle()) {
            auto lineStyle = style->get_linestyle();
            if (lineStyle->has_color()) {
                lineColor = KmlUtils::qcolorFromKmlColor(lineStyle->get_color());
            }
            if (lineStyle->has_width()) {
                lineWidth = lineStyle->get_width();
            }
        }
    }
    _polyline.setLineColor(lineColor);
    _polyline.setLineWidth(lineWidth);
    _contextMenu = new QMenu(mapRect()->parent());
    _removeAction = _contextMenu->addAction("Удалить точку");
    _contextMenu->addSeparator();
    _copyAction = _contextMenu->addAction("Скопировать координаты");

    connect(_removeAction, &QAction::triggered, this, [this]() {
        removeActive();
        emit sceneUpdated();
    });
    connect(_copyAction, &QAction::triggered, this,
            [this]() { _polyline.at(_activeFlag.unwrap()).printCoordinatesToClipboard(mapRect()); });
}

KmlPolyLineLayer::~KmlPolyLineLayer()
{
}

void KmlPolyLineLayer::removeActive()
{
    _coordinates->remove_at(_activeFlag.unwrap());
    _polyline.remove(_activeFlag.unwrap());
    _activeFlag = misc::None;
}

bool KmlPolyLineLayer::canAddPoints() const
{
    return true;
}

KmlElementLayer::Type KmlPolyLineLayer::type() const
{
    return KmlElementLayer::LineString;
}

void KmlPolyLineLayer::setActive(bool isActive)
{
    _isActive = isActive;
}

void KmlPolyLineLayer::draw(QPainter* p) const
{
    _polyline.drawLines(p);
    if (_isActive) {
        _polyline.drawPoints(p);
    }
    QPen pen;
    pen.setColor(Qt::black);
    p->setPen(pen);
    if (_activeFlag.isSome()) {
        QPointF offset = _activePixmap.rect().center();
        QPointF pos = _polyline.at(_activeFlag.unwrap()).position();
        drawCoordinatesAt(p, pos, pos + QPointF(offset.x(), -offset.y()));
    }
    if (_activeLine.isSome()) {
        QPointF offset = _activePixmap.rect().center();
        QPointF pos = _activeLine.unwrap().pos;
        p->drawPixmap(pos - offset - mapRect()->mapOffsetRaw(), _activePixmap);
        drawCoordinatesAt(p, pos, pos + QPointF(offset.x(), -offset.y()));
    }
}

bool KmlPolyLineLayer::hasElementAt(const QPointF& pos) const
{
    return _polyline.nearestLine(pos, 5).isSome();
}

bool KmlPolyLineLayer::zoomEvent(const QPoint& pos, int fromZoom, int toZoom)
{
    (void)pos;
    _polyline.changeZoomLevel(fromZoom, toZoom);
    return true;
}

void KmlPolyLineLayer::changeProjection(const MercatorProjection& from, const MercatorProjection& to)
{
    _polyline.changeProjection(from, to);
}

bool KmlPolyLineLayer::viewportResetEvent(int oldZoom, int newZoom, const QRect& oldViewpiort, const QRect& newViewport)
{
    (void)oldViewpiort;
    (void)newViewport;
    _polyline.changeZoomLevel(oldZoom, newZoom);
    return true;
}

void KmlPolyLineLayer::updateKml()
{
}

void KmlPolyLineLayer::addFlagAt(const QPointF& pos)
{
    insertFlagAt(_polyline.count(), pos);
}

void KmlPolyLineLayer::insertFlagAt(std::size_t index, const QPointF& pos)
{
    _polyline.insert(index, createMarker(pos));
    core::LatLon latLon = WithPosition<>::toLatLon(pos, mapRect());
    _coordinates->insert_latlng(index, latLon.latitude, latLon.longitude);
}

misc::Option<std::size_t> KmlPolyLineLayer::flagAt(const QPointF& pos)
{
    return _polyline.nearest(pos);
}

misc::Option<LineIndexAndPos> KmlPolyLineLayer::lineAt(const QPointF& pos)
{
    return _polyline.nearestLine(pos, 5);
}

void KmlPolyLineLayer::moveCurrentBy(const QPointF& delta)
{
    std::size_t index = _activeFlag.unwrap();
    _polyline.moveBy(index, delta);
    core::LatLon latLon = _polyline.at(index).toLatLon(mapRect());
    _coordinates->set_coordinates_array_at(kmlbase::Vec3(latLon.longitude, latLon.latitude), index);
}

void KmlPolyLineLayer::setCurrentActive(bool isActive)
{
    _polyline.at(_activeFlag.unwrap()).setActive(isActive);
}

bool KmlPolyLineLayer::showContextMenuForCurrent(const QPoint& pos)
{
    _contextMenu->exec(mapRect()->toWindowSystemCoordinates(pos));
    return true;
}

bool KmlPolyLineLayer::showContextMenuForNone(const QPoint& pos)
{
    (void)pos;
    return false;
}

void KmlPolyLineLayer::showFlagEditor(const QPoint& pos)
{
    (void)pos;
}

void KmlPolyLineLayer::finishMovingCurrent()
{
}
}
}
}
