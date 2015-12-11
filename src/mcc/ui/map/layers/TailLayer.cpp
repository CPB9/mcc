/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "mcc/ui/map/layers/TailLayer.h"
#include "mcc/ui/map/drawables/Flag.h"

#include <QColor>
#include <QTime>
#include <QDebug>

namespace mcc {
namespace ui {
namespace map {

TailLayer::TailLayer(const MapRectConstPtr& mapRect, double tailParam, core::TrailMode mode)
    : Layer(mapRect)
    , _tail(mapRect)
    , _mode(mode)
    , _maxParam(tailParam)
{
    QImage img = Flag::drawFlag(20, 30, Qt::darkRed);
    _lostMarker.setPixmap(QPixmap::fromImage(img));
    _lostMarker.setAlignment(Qt::AlignVCenter | Qt::AlignBottom);
}

TailLayer::~TailLayer()
{
}

bool TailLayer::mouseMoveEvent(const QPoint& oldPos, const QPoint& newPos)
{
    (void)oldPos;
    (void)newPos;
    return false;
}

bool TailLayer::mousePressEvent(const QPoint& pos)
{
    (void)pos;
    return false;
}

bool TailLayer::mouseReleaseEvent(const QPoint& pos)
{
    (void)pos;
    return false;
}

bool TailLayer::contextMenuEvent(const QPoint& pos)
{
    (void)pos;
    return false;
}

bool TailLayer::viewportResizeEvent(const QSize& oldSize, const QSize& newSize)
{
    (void)oldSize;
    (void)newSize;
    return false;
}

bool TailLayer::viewportScrollEvent(const QPoint& oldPos, const QPoint& newPos)
{
    (void)oldPos;
    (void)newPos;
    return false;
}

bool TailLayer::zoomEvent(const QPoint& pos, int fromZoom, int toZoom)
{
    (void)pos;
    _tail.changeZoomLevel(fromZoom, toZoom);
    for (LostSignalMarker& marker : _markers) {
        marker.changeZoomLevel(fromZoom, toZoom);
    }
    return true;
}

void TailLayer::changeProjection(const MercatorProjection& from, const MercatorProjection& to)
{
    _tail.changeProjection(from, to);
    for (LostSignalMarker& marker : _markers) {
        marker.changeProjection(mapRect(), from, to);
    }
}

void TailLayer::draw(QPainter* p) const
{
    if (_tail.count() > 1) {
        QPen lostPen;
        lostPen.setColor(_tail.lineColor());
        lostPen.setWidthF(_tail.lineWidth());
        QPen normalPen = lostPen;
        lostPen.setStyle(Qt::DashLine);
        normalPen.setStyle(Qt::SolidLine);

        auto currentStorage = _storage.end() - _tail.count();
        auto current = _tail.points().begin();
        bool lastIsLost = currentStorage->lostSignal;
        if (lastIsLost) {
            p->setPen(lostPen);
        } else {
            p->setPen(normalPen);
        }
        for (auto next = (_tail.points().begin() + 1); next < _tail.points().end(); next++) {
            bool currentIsLost = currentStorage->lostSignal;
            const QPoint& offset = mapRect()->mapOffsetRaw();
            if (currentIsLost) {
                if (!lastIsLost) {
                    p->setPen(lostPen);
                }
                p->drawLine(current->position() - offset, next->position() - offset);
            } else {
                if (lastIsLost) {
                    p->setPen(normalPen);
                }
                p->drawLine(current->position() - offset, next->position() - offset);
            }
            lastIsLost = currentIsLost;
            current = next;
            currentStorage++;
        }
        for (auto it = _markers.begin(); it < _markers.end(); it++) {
            it->draw(p, _lostMarker.pixmap(), mapRect());
        }
    }
}

void TailLayer::setLineColor(const QColor& color)
{
    _tail.setLineColor(color);
    emit sceneUpdated();
}

void TailLayer::setLineWidth(int width)
{
    _tail.setLineWidth(width);
    emit sceneUpdated();
}

void TailLayer::addPointNoCut(const core::GeoPosition& position, const QDateTime& time)
{
    (void)time;
    // auto stdTime = std::chrono::system_clock::from_time_t(time.toTime_t()); // неправильное время приходит
    auto stdTime = std::chrono::system_clock::now();
    addPointNoCut(position, stdTime);
}

void TailLayer::addPointNoCut(const core::GeoPosition& position, const std::chrono::system_clock::time_point& time)
{
    _storage.emplace_back(position, time);
    if (_mode != core::TrailMode::None) {
        _tail.emplaceBack(WithPosition<>::fromLatLon(core::LatLon(position.latitude, position.longitude), mapRect()));
    }
}

void TailLayer::addPoint(const core::GeoPosition& position, const QDateTime& time)
{
    addPointNoCut(position, time);
    cut();
    emit sceneUpdated();
}

void TailLayer::cut()
{
    if (_mode == core::TrailMode::Distance) {
        cutToDistance();
    } else if (_mode == core::TrailMode::Time) {
        cutToTime();
    }
}

void TailLayer::cutToDistance()
{
    if (_tail.points().empty())
        return;

    auto start = _tail.points().begin();
    auto current = start;
    auto end = _tail.points().end() - 1;
    double totalDistance = _tail.totalDistance();
    std::size_t count = 0;

    while ((totalDistance > _maxParam) && (current < end)) {
        totalDistance -= current->distance();
        current++;
        count++;
    }

    if (count) {
        _tail.removeFront(count);
        cutMarkers();
    }
}

void TailLayer::cutToTime()
{
    if (_storage.empty()) {
        return;
    }
    auto start = _storage.end() - _tail.count();
    auto current = start;
    auto end = _storage.end();
    std::size_t count = 0;
    std::chrono::system_clock::time_point lastTime = _storage.back().time;
    std::chrono::seconds delta((int64_t)_maxParam);
    std::chrono::system_clock::time_point minimum = lastTime - delta;
    while ((current->time < minimum) && (current < end)) {
        current++;
        count++;
    }

    _tail.removeFront(count);
    cutMarkers();
}

void TailLayer::cutMarkers()
{
    if (_tail.count() == 0) {
        return;
    }
    std::chrono::system_clock::time_point time = (_storage.end() - _tail.count())->time;
    auto it = _markers.begin();
    for (; it < _markers.end(); it++) {
        if (time <= it->time()) {
            break;
        }
    }
    _markers.erase(_markers.begin(), it);
}

void TailLayer::setMaxTailParam(double param)
{
    setTrailModeAndParam(_mode, param);
}

void TailLayer::setTrailMode(core::TrailMode mode)
{
    setTrailModeAndParam(mode, _maxParam);
}

void TailLayer::setSignalLost()
{
    if (!_storage.empty()) {
        TailPointInfo& last = _storage.back();
        if (!last.lostSignal) {
            core::LatLon latLon = core::LatLon(last.position.latitude, last.position.longitude);
            _markers.emplace_back(latLon, last.time, mapRect());
        }
        last.setSignalLost(true);
    }
}

void TailLayer::setTrailModeAndParam(core::TrailMode mode, double param)
{
    // TODO: оптимизировать
    _mode = mode;
    _maxParam = param;
    _tail.clear();
    _markers.clear();

    if (_mode != core::TrailMode::None) {
        for (const TailPointInfo& point : _storage) {
            core::LatLon latLon = core::LatLon(point.position.latitude, point.position.longitude);
            QPointF pos = WithPosition<>::fromLatLon(latLon, mapRect());
            _tail.emplaceBack(pos);
            if (point.signalIsLost()) {
                _markers.emplace_back(pos, latLon, point.time);
            }
        }
        cut();
    }
    emit sceneUpdated();
}

bool TailLayer::viewportResetEvent(int oldZoom, int newZoom, const QRect& oldViewpiort, const QRect& newViewport)
{
    (void)oldViewpiort;
    (void)newViewport;
    _tail.changeZoomLevel(oldZoom, newZoom);
    return true;
}

void TailLayer::clear()
{
    _tail.clear();
    emit sceneUpdated();
}

}
}
}
