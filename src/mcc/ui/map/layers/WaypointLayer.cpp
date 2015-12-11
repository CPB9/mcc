/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "mcc/ui/map/layers/WaypointLayer.h"
#include <drawables/Flag.h>
#include "mcc/ui/core/Route.h"
#include "mcc/ui/core/FlyingDevice.h"

#include <QMenu>
#include <QAction>

namespace mcc {
namespace ui {
namespace map {

using mcc::ui::core::Waypoint;
using mcc::ui::core::Route;
using mcc::ui::core::FlyingDevice;

WaypointLayer::WaypointLayer(mcc::ui::core::FlyingDevice* device, mcc::ui::core::Route* route, const MapRectConstPtr& rect)
    : SimpleFlagLayer(rect)
    , _device(device)
    , _model(route)
    , _markers(rect)
    , _style(Inactive)
    , _catchUpdates(true)
    , _lastTargetWaypoint(device->nextWaypoint())
{
    connect(_model, &Route::visibilityChanged, this, [this](bool) { emit sceneUpdated(); });
    connect(_model, &Route::styleChagned, this, [this]() { setStyle(_style); });
    connect(_device, &FlyingDevice::nextWaypointChanged, this, [this](int index) {
        if (_device->activeRoute()->id() != _model->id())
            return;

        updateMarker(_lastTargetWaypoint);
        updateMarker(index);
        _lastTargetWaypoint = index;
    });
    connect(_model, &Route::waypointChanged, this, [this](const Waypoint& waypoint, int index) {
        if (_catchUpdates) {
            _markers.setLatLon(index, core::LatLon(waypoint.position.latitude, waypoint.position.longitude));
            updateMarker(index);
            emit sceneUpdated();
        }
    });
    connect(_model, &Route::waypointMoved, this, [this](int from, int to) {
        if (_catchUpdates) {
            _markers.swap(from, to);
            updateMarker(from);
            updateMarker(to);
            emit sceneUpdated();
        }
    });
    connect(_model, &Route::waypointRemoved, this, [this](int index) {
        if (_catchUpdates) {
            _markers.remove(index);
            repaintMarkerPixmapsFrom(index);
            emit sceneUpdated();
        }
    });
    connect(_model, &Route::waypointInserted, this, [this](const Waypoint& waypoint, int index) {
        if (_catchUpdates) {
            _markers.insert(index, createMarker(waypoint, index));
            emit sceneUpdated();
        }
    });
    connect(_model, &Route::routeChanged, this, [this]() {
        if (_catchUpdates) {
            resetWaypoints();
            emit sceneUpdated();
        }
    });
    connect(_model, &Route::ringModeChanged, this, [this](bool isRing) {
        if (_catchUpdates) {
            _markers.setConnected(isRing);
            emit sceneUpdated();
        }
    });

    _contextMenu = new QMenu(mapRect()->parent());
    _removeAction = _contextMenu->addAction("Удалить точку");
    _contextMenu->addSeparator();
    _copyAction = _contextMenu->addAction("Скопировать координаты");

    connect(_removeAction, &QAction::triggered, this, [this]() {
        removeActive();
        emit sceneUpdated();
    });
    connect(_copyAction, &QAction::triggered, this,
            [this]() { _markers.at(_activeFlag.unwrap()).printCoordinatesToClipboard(mapRect()); });
    createPlaceholders();
    _markers.setConnected(_model->ring());
    resetWaypoints();
    emit sceneUpdated();
}

WaypointLayer::~WaypointLayer()
{
}

void WaypointLayer::updateMarker(std::size_t index)
{
    if (index >= _model->waypointsCount())
        return;

    updateMarkerPixmap(&_markers.at(index), index, _model->waypointAt(index).flags);
}

void WaypointLayer::removeActive()
{
    std::size_t index = _activeFlag.unwrap();
    _markers.remove(index);
    _catchUpdates = false;
    _model->removeWaypoint(index);
    _catchUpdates = true;
    repaintMarkerPixmapsFrom(index);
    _activeFlag = misc::None;
}

static int flagWidth = 61;

void WaypointLayer::createPlaceholders()
{
    _emptyPixmap = QPixmap::fromImage(Flag::drawWaypointFlag(flagWidth, flagWidth, Qt::yellow));
}

QPixmap WaypointLayer::createPixmap(std::size_t index, bool active, core::WaypointFlags flags) const
{
    QColor color;
    switch (_style) {
    case Inactive:
        if (index == (std::size_t)_device->nextWaypoint()) {
            color = Qt::black;
        } else {
            color = Qt::gray;
        }
        break;
    case Active:
        if (index == (std::size_t)_device->nextWaypoint()) {
            color = _model->activePointColor();
        } else {
            color = _model->inactivePointColor();
        }
        break;
    case Editable:
        if (active) {
            color = Qt::yellow;
        } else {
            color = Qt::green;
        }
        break;
    }
    return QPixmap::fromImage(Flag::drawWaypointFlag(flagWidth, flagWidth, color, index + 1, flags));
}

BiMarker WaypointLayer::createMarker(const Waypoint& waypoint, std::size_t index) const
{
    core::LatLon latLon(waypoint.position.latitude, waypoint.position.longitude);
    QPointF pos = WithPosition<>::fromLatLon(latLon, mapRect());
    return createMarker(pos, index, waypoint.flags);
}

void WaypointLayer::updateMarkerPixmap(BiMarker* marker, std::size_t index, core::WaypointFlags flags) const
{
    QPixmap inactive = createPixmap(index, false, flags);
    marker->setInactivePixmap(inactive);
    if (_style == Editable) {
        QPixmap active = createPixmap(index, true, flags);
        marker->setActivePixmap(active);
    } else {
        marker->setActivePixmap(inactive);
    }
}

BiMarker WaypointLayer::createMarker(const QPointF& pos, std::size_t index, core::WaypointFlags flags) const
{
    BiMarker m(pos);
    m.setActiveAlignment(Qt::AlignBottom | Qt::AlignHCenter);
    m.setInactiveAlignment(Qt::AlignBottom | Qt::AlignHCenter);
    updateMarkerPixmap(&m, index, flags);
    return m;
}

void WaypointLayer::resetWaypoints()
{
    _markers.clear();
    const core::WaypointsList& waypoints = _model->waypointsList();
    std::size_t i = 0;
    for (const Waypoint& wp : waypoints) {
        _markers.append(createMarker(wp, i));
        i++;
    }
}

void WaypointLayer::setStyle(WaypointLayer::Style style)
{
    _style = style;
    switch (style) {
    case Inactive:
        _markers.setLineColor(Qt::gray);
        _markers.setLineWidth(1);
        break;
    case Active:
        _markers.setLineColor(_model->pen().color());
        _markers.setLineWidth(_model->pen().widthF());
        break;
    case Editable:
        _markers.setLineColor(Qt::red);
        _markers.setLineWidth(3);
        break;
    }
    createPlaceholders();
    resetWaypoints();
    emit sceneUpdated();
}

void WaypointLayer::repaintMarkerPixmapsFrom(std::size_t start)
{
    if (start >= _markers.count()) {
        return;
    }
    for (std::size_t i = start; i < _markers.count(); i++) {
        updateMarker(i);
    }
}

void WaypointLayer::draw(QPainter* p) const
{
    if (!_model->visible()) {
        return;
    }
    _markers.draw(p);
    QPen pen;
    pen.setColor(Qt::black);
    p->setPen(pen);
    if (_activeFlag.isSome() || _activeLine.isSome()) {
        double distance;
        QPointF pos;
        QRectF rect = _emptyPixmap.rect();
        QPointF offset = QPointF(rect.center().x(), rect.bottom());
        if (_activeFlag.isSome()) {
            if (_activeFlag.unwrap() == (_markers.count() - 1)) {
                distance = _markers.totalDistance();
            } else {
                distance = _markers.distanceTo(_activeFlag.unwrap());
            }
            pos = _markers.at(_activeFlag.unwrap()).position();
        } else {
            pos = _activeLine.unwrap().pos;
            distance = _markers.distanceTo(_activeLine.unwrap().index, pos);
            p->drawPixmap(pos - offset - mapRect()->mapOffsetRaw(), _emptyPixmap);
        }
        const char* suffix = "м";
        if (distance > 1000) {
            distance /= 1000;
            suffix = "км";
        }
        QString dtext = (QString("%1") + suffix).arg(distance, 0, 'f', 2) + " (%1, %2)";
        drawCoordinatesAt(p, pos, pos + QPointF(offset.x(), -offset.y()), dtext);
    }
}

bool WaypointLayer::viewportResetEvent(int oldZoom, int newZoom, const QRect& oldViewpiort, const QRect& newViewport)
{
    (void)oldViewpiort;
    (void)newViewport;
    _markers.changeZoomLevel(oldZoom, newZoom);
    return true;
}

bool WaypointLayer::zoomEvent(const QPoint& pos, int fromZoom, int toZoom)
{
    (void)pos;
    _markers.changeZoomLevel(fromZoom, toZoom);
    return true;
}

void WaypointLayer::changeProjection(const MercatorProjection& from, const MercatorProjection& to)
{
    _markers.changeProjection(from, to);
}

void WaypointLayer::addFlagAt(const QPointF& pos)
{
    insertFlagAt(_markers.count(), pos);
}

void WaypointLayer::insertFlagAt(std::size_t index, const QPointF& pos)
{
    core::LatLon latLon = WithPosition<>::toLatLon(pos, mapRect());
    core::Waypoint waypoint(latLon.latitude, latLon.longitude, 0, 0);
    auto marker = createMarker(pos, index + 1);
    marker.setActive(true);
    _markers.insert(index, std::move(marker));
    _catchUpdates = false;
    if (_model->waypointsCount() > 0)
    {
        const auto& wp = _model->waypointsList().last();
        waypoint.position.altitude = wp.position.altitude;
        waypoint.speed = wp.speed;
    }
    _model->insertWaypoint(waypoint, index);
    _catchUpdates = true;
    _activeFlag = _markers.count() - 1;
    repaintMarkerPixmapsFrom(index);
}

void WaypointLayer::finishMovingCurrent()
{
}

misc::Option<std::size_t> WaypointLayer::flagAt(const QPointF& pos)
{
    return _markers.nearest(pos);
}

misc::Option<LineIndexAndPos> WaypointLayer::lineAt(const QPointF& pos)
{
    return _markers.nearestLine(pos, 5);
}

void WaypointLayer::moveCurrentBy(const QPointF& delta)
{
    std::size_t index = _activeFlag.unwrap();
    _markers.moveBy(index, delta);
    _catchUpdates = false;
    Waypoint wp = _model->waypointAt(index);
    core::LatLon latLon = _markers.at(index).toLatLon(mapRect());
    wp.position.latitude = latLon.latitude;
    wp.position.longitude = latLon.longitude;
    _model->setWaypoint(wp, index);
    _catchUpdates = true;
}

void WaypointLayer::setCurrentActive(bool isActive)
{
    _markers.at(_activeFlag.unwrap()).setActive(isActive);
}

bool WaypointLayer::showContextMenuForCurrent(const QPoint& pos)
{
    _contextMenu->exec(mapRect()->toWindowSystemCoordinates(pos));
    return true;
}

bool WaypointLayer::showContextMenuForNone(const QPoint& pos)
{
    (void)pos;
    return false;
}

void WaypointLayer::showFlagEditor(const QPoint& pos)
{
    (void)pos;
}
}
}
}
