/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#pragma once

#include "mcc/ui/map/drawables/Interfaces.h"
#include "mcc/ui/map/drawables/Wrappers.h"
#include "mcc/ui/map/MapRect.h"
#include "mcc/ui/map/LineAndPos.h"
#include "mcc/misc/Option.h"
#include "mcc/ui/core/Structs.h"

#include <QPointF>
#include <QColor>
#include <QPainter>
#include <QDebug>

#include <deque>

namespace mcc {
namespace ui {
namespace map {

template <typename T, typename W = DistanceWrapper<AngleWrapper<EmptyWrapper<T>>>>
class PolyLineBase {
public:
    typedef W ElementType;
    PolyLineBase(const MapRectConstPtr& rect);

    misc::Option<std::size_t> nearest(const QPointF& position) const;
    misc::Option<LineIndexAndPos> nearestLine(const QPointF& position, int width, bool isConnected = false) const;
    QPointF positionBetween(std::size_t frist, std::size_t second) const;
    core::GeoBox geoBox() const;
    bool hasPointInside(const QPointF& pos) const;

    const MapRectConstPtr& mapRect();

    void clear();
    void remove(std::size_t index);
    void removeFront();
    void removeFront(std::size_t count);

    void set(std::size_t index, T&& point);
    void append(T&& point);
    void insert(std::size_t index, T&& point);
    void swap(std::size_t first, std::size_t second);

    template <typename... A>
    void emplaceBack(A&&... args);
    template <typename... A>
    void emplace(std::size_t index, A&&... args);

    void moveBy(std::size_t index, const QPointF& pos);
    void moveAll(const QPointF& pos);
    void setPosition(std::size_t index, const QPointF& pos);
    void setLatLon(std::size_t index, const core::LatLon& latLon);

    std::size_t count() const;
    const W& at(std::size_t index) const;
    W& at(std::size_t index);
    const W& last() const;

    double totalDistance(bool isConnected = false) const;
    double distanceTo(std::size_t index) const;
    double distanceTo(std::size_t index, const QPointF& next) const;

    void changeProjection(const MercatorProjection& from, const MercatorProjection& to);
    void changeZoomLevel(int from, int to);

    void setLineWidth(double width);
    void setLineColor(const QColor& color);
    void setFillColor(const QColor& color);

    double lineWidth() const;
    const QColor& lineColor() const;
    const QColor& fillColor() const;

    void drawLines(QPainter* p) const;
    void drawLinesConnected(QPainter* p) const;
    void drawPolygon(QPainter* p) const;
    void drawPoints(QPainter* p) const;
    const std::deque<W>& points() const;
    const MapRectConstPtr& mapRect() const
    {
        return _rect;
    }

    template <typename I>
    void drawArrowheadBetween(I it1, I it2, QPainterPath* path) const;
    void drawArrowhead(const QPointF& pos, double angle, QPainterPath* path) const;
    QPainterPath createArrowheads(bool isConnected = false) const;

private:
    void recalcDistancesAround(std::size_t index);
    void recalcDistancesFor(std::size_t current, std::size_t next);

    std::deque<W> _points;
    MapRectConstPtr _rect;
    double _totalDistance;
    double _lineWidth;
    QColor _lineColor;
    QColor _fillColor;
};

template <typename T, typename W>
inline PolyLineBase<T, W>::PolyLineBase(const MapRectConstPtr& rect)
    : _rect(rect)
    , _totalDistance(0)
    , _lineWidth(1)
    , _lineColor(Qt::white)
{
}

template <typename T, typename W>
inline const MapRectConstPtr& PolyLineBase<T, W>::mapRect()
{
    return _rect;
}

template <typename T, typename W>
misc::Option<std::size_t> PolyLineBase<T, W>::nearest(const QPointF& position) const
{
    std::size_t i = 0;
    for (const W& point : _points) {
        if (point.hasInRect(position)) {
            return i;
        }
        i++;
    }
    return misc::None;
}

template <typename T, typename W>
double PolyLineBase<T, W>::distanceTo(std::size_t index) const
{
    double distance = 0;
    for (auto it = _points.begin(); it < _points.begin() + index; it++) {
        distance += it->distance();
    }
    return distance;
}

template <typename T, typename W>
double PolyLineBase<T, W>::distanceTo(std::size_t index, const QPointF& next) const
{
    double distance = distanceTo(index);
    core::LatLon latLon1 = _points[index].toLatLon(_rect);
    core::LatLon latLon2 = WithPosition<>::toLatLon(next, _rect);
    return distance + _rect->projection().calcDistance(latLon1, latLon2);
}

template <typename T, typename W>
misc::Option<LineIndexAndPos> PolyLineBase<T, W>::nearestLine(const QPointF& position, int width,
                                                              bool isConnected) const
{
    if (_points.size() < 2) {
        return misc::None;
    }

    auto projectPointOnLine = [](double x1, double y1, double x2, double y2, double x3, double y3) {
        // возможны переполнения, сделать через скалярное произведение
        double dx = x2 - x1;
        double dy = y2 - y1;
        double k = (dy * (x3 - x1) - dx * (y3 - y1)) / (dy * dy + dx * dx);
        double x4 = x3 - k * dy;
        double y4 = y3 + k * dx;
        return QPointF(x4, y4);
    };

    auto projectPointOnLine2 = [&projectPointOnLine](const QPointF& p1, const QPointF& p2, const QPointF& p3) {
        return projectPointOnLine(p1.x(), p1.y(), p2.x(), p2.y(), p3.x(), p3.y());
    };

    auto it = _points.begin();
    auto nextIt = it + 1;
    while (nextIt < _points.end()) {
        const QPointF& p1 = it->position();
        const QPointF& p2 = nextIt->position();
        QRectF rect(p1, p2);
        if (rect.contains(position)) {
            QPointF projected = projectPointOnLine2(p1, p2, position);
            if (std::hypot(position.x() - projected.x(), position.y() - projected.y()) < width) {
                return misc::Option<LineIndexAndPos>(misc::InPlace, it - _points.begin(), projected);
            }
        }
        it = nextIt;
        nextIt++;
    }
    if (isConnected) {
        it = _points.end() - 1;
        nextIt = _points.begin();
        const QPointF& p1 = it->position();
        const QPointF& p2 = nextIt->position();
        QRectF rect(p1, p2);
        if (rect.contains(position)) {
            QPointF projected = projectPointOnLine2(p1, p2, position);
            if (std::hypot(position.x() - projected.x(), position.y() - projected.y()) < width) {
                return misc::Option<LineIndexAndPos>(misc::InPlace, it - _points.begin(), projected);
            }
        }
    }
    return misc::None;
}

template <typename T, typename W>
inline QPointF PolyLineBase<T, W>::positionBetween(std::size_t frist, std::size_t second) const
{
    return (_points[frist].position() + _points[second].position()) / 2;
}

// http://geomalgorithms.com/a03-_inclusion.html
// crossing number test

template <typename T, typename W>
bool PolyLineBase<T, W>::hasPointInside(const QPointF& pos) const
{
    if (_points.size() < 3) {
        return false;
    }

    int cn = 0;
    auto incCrossingNumber = [&cn, &pos](const QPointF& currentPos, const QPointF& nextPos) {
        if (((currentPos.y() <= pos.y()) && (nextPos.y() > pos.y()))
            || ((currentPos.y() > pos.y()) && (nextPos.y() <= pos.y()))) {
            double vt = (pos.y() - currentPos.y()) / (nextPos.y() - currentPos.y());
            if (pos.x() < currentPos.x() + vt * (nextPos.x() - currentPos.x())) {
                cn++;
            }
        }
    };

    auto it = _points.begin();
    while (it < (_points.end() - 1)) {
        incCrossingNumber(it->position(), (it + 1)->position());
        it++;
    }
    incCrossingNumber((_points.end() - 1)->position(), _points.begin()->position());
    return (cn & 1);
}

template <typename T, typename W>
void PolyLineBase<T, W>::recalcDistancesFor(std::size_t currentIndex, std::size_t nextIndex)
{
    W& cur = _points[currentIndex];
    const W& next = _points[nextIndex];
    _totalDistance -= cur.distance();
    cur.update(next, _rect);
    _totalDistance += cur.distance();
}

template <typename T, typename W>
void PolyLineBase<T, W>::recalcDistancesAround(std::size_t index)
{
    std::size_t lastIndex = _points.size() - 1;
    if (_points.size() <= 1) {
        _totalDistance = 0;
        return;
    } else if (index > lastIndex) {
        index = lastIndex;
    }
    if (index == 0) {
        recalcDistancesFor(lastIndex, 0);
    } else {
        recalcDistancesFor(index - 1, index);
    }
    if (index == lastIndex) {
        recalcDistancesFor(lastIndex, 0);
    } else {
        recalcDistancesFor(index, index + 1);
    }
}

template <typename T, typename W>
inline void PolyLineBase<T, W>::clear()
{
    _totalDistance = 0;
    _points.clear();
}

template <typename T, typename W>
void PolyLineBase<T, W>::remove(std::size_t index)
{
    double distance = _points[index].distance();
    _points.erase(_points.begin() + index);
    if (_points.empty()) {
        _totalDistance = 0;
    } else if (_points.size() == 1) {
        _totalDistance = 0;
        _points[0].setSingle();
    } else {
        _totalDistance -= distance;
        recalcDistancesAround(index);
    }
}

template <typename T, typename W>
void PolyLineBase<T, W>::removeFront()
{
    double distance = _points[0].distance();
    _points.pop_front();
    if (_points.empty()) {
        _totalDistance = 0;
    } else if (_points.size() == 1) {
        _totalDistance = 0;
        _points[0].setSingle();
    } else {
        _totalDistance -= distance;
        recalcDistancesFor(_points.size() - 1, 0);
    }
}

template <typename T, typename W>
void PolyLineBase<T, W>::removeFront(std::size_t count)
{
    if (count == 0) {
        return;
    }
    if (count >= _points.size()) {
        _points.clear();
        _totalDistance = 0;
        return;
    }
    auto start = _points.begin();
    auto current = start;
    auto end = start + count;
    while (current < end) {
        _totalDistance -= current->distance();
        current++;
    }
    _points.erase(start, start + count);
    if (_points.size() == 1) {
        _totalDistance = 0;
        _points[0].setSingle();
    }
}

template <typename T, typename W>
inline void PolyLineBase<T, W>::set(std::size_t index, T&& point)
{
    _points[index].reset(std::forward<T>(point));
    recalcDistancesAround(index);
}

template <typename T, typename W>
inline void PolyLineBase<T, W>::setLatLon(std::size_t index, const core::LatLon& latLon)
{
    _points[index].setLatLon(latLon, _rect);
    recalcDistancesAround(index);
}

template <typename T, typename W>
inline void PolyLineBase<T, W>::moveBy(std::size_t index, const QPointF& delta)
{
    _points[index].moveBy(delta);
    recalcDistancesAround(index);
}

template <typename T, typename W>
inline void PolyLineBase<T, W>::moveAll(const QPointF& delta)
{
    for (W& point : _points) {
        point.moveBy(delta);
    }
    // TODO: recalcDistances
}

template <typename T, typename W>
inline void PolyLineBase<T, W>::setPosition(std::size_t index, const QPointF& pos)
{
    _points[index].setPosition(pos);
    recalcDistancesAround(index);
}

template <typename T, typename W>
void PolyLineBase<T, W>::append(T&& point)
{
    _points.emplace_back(bmcl::InPlace, std::forward<T>(point));
    recalcDistancesAround(_points.size() - 1);
}

template <typename T, typename W>
inline void PolyLineBase<T, W>::insert(std::size_t index, T&& point)
{
	_points.emplace(_points.begin() + index, bmcl::InPlace, std::forward<T>(point));
    recalcDistancesAround(index);
}

template <typename T, typename W>
inline void PolyLineBase<T, W>::swap(std::size_t first, std::size_t second)
{
    auto it1 = _points.begin() + first;
    auto it2 = _points.begin() + second;
    std::swap(*it1, *it2);
    recalcDistancesAround(first);
    recalcDistancesAround(second);
}

template <typename T, typename W>
inline std::size_t PolyLineBase<T, W>::count() const
{
    return _points.size();
}

template <typename T, typename W>
inline W& PolyLineBase<T, W>::at(std::size_t index)
{
    return _points[index];
}

template <typename T, typename W>
inline const W& PolyLineBase<T, W>::at(std::size_t index) const
{
    return _points[index];
}

template <typename T, typename W>
inline const W& PolyLineBase<T, W>::last() const
{
    return _points.back();
}

template <typename T, typename W>
double PolyLineBase<T, W>::totalDistance(bool isConnected) const
{
    if (!isConnected && _points.size() >= 2) {
        return _totalDistance - _points.back().distance();
    }
    return _totalDistance;
}

template <typename T, typename W>
template <typename... A>
inline void PolyLineBase<T, W>::emplace(std::size_t index, A&&... args)
{
	_points.emplace(_points.begin() + index, bmcl::InPlace, std::forward<A>(args)...);
    recalcDistancesAround(index);
}

template <typename T, typename W>
template <typename... A>
inline void PolyLineBase<T, W>::emplaceBack(A&&... args)
{
	_points.emplace_back(bmcl::InPlace, std::forward<A>(args)...);
    recalcDistancesAround(_points.size() - 1);
}

template <typename T, typename W>
inline void PolyLineBase<T, W>::changeProjection(const MercatorProjection& from, const MercatorProjection& to)
{
    for (W& point : _points) {
        point.changeProjection(_rect, from, to);
    }
}

template <typename T, typename W>
inline void PolyLineBase<T, W>::changeZoomLevel(int from, int to)
{
    for (W& point : _points) {
        point.changeZoomLevel(from, to);
    }
}

template <typename T, typename W>
inline void PolyLineBase<T, W>::setLineWidth(double width)
{
    _lineWidth = width;
}

template <typename T, typename W>
inline void PolyLineBase<T, W>::setLineColor(const QColor& color)
{
    _lineColor = color;
}

template <typename T, typename W>
inline void PolyLineBase<T, W>::setFillColor(const QColor& color)
{
    _fillColor = color;
}

template <typename T, typename W>
inline double PolyLineBase<T, W>::lineWidth() const
{
    return _lineWidth;
}

template <typename T, typename W>
inline const QColor& PolyLineBase<T, W>::lineColor() const
{
    return _lineColor;
}

template <typename T, typename W>
inline const QColor& PolyLineBase<T, W>::fillColor() const
{
    return _fillColor;
}

template <typename T, typename W>
void PolyLineBase<T, W>::drawLines(QPainter* p) const
{
    QPen pen;
    pen.setColor(_lineColor);
    pen.setWidthF(_lineWidth);
    p->setPen(pen);
    if (_points.size() > 1) {
        auto current = _points.begin();
        const QPoint& offset = _rect->mapOffsetRaw();
        for (auto it = (_points.begin() + 1); it < _points.end(); it++) {
            p->drawLine(current->position() - offset, it->position() - offset);
            current = it;
        }
    }
}

template <typename T, typename W>
void PolyLineBase<T, W>::drawLinesConnected(QPainter* p) const
{
    QPen pen;
    pen.setColor(_lineColor);
    pen.setWidthF(_lineWidth);
    p->setPen(pen);
    if (_points.size() > 1) {
        auto current = _points.begin();
        const QPoint& offset = _rect->mapOffsetRaw();
        for (auto it = (_points.begin() + 1); it < _points.end(); it++) {
            p->drawLine(current->position() - offset, it->position() - offset);
            current = it;
        }
        p->drawLine(current->position() - offset, _points.begin()->position() - offset);
    }
}

template <typename T, typename W>
void PolyLineBase<T, W>::drawPolygon(QPainter* p) const
{
    QPainterPath path;
    p->setBrush(_fillColor);
    QPen pen;
    pen.setColor(_lineColor);
    pen.setWidthF(_lineWidth);
    p->setPen(pen);
    if (_points.size() > 1) {
        const QPoint& offset = _rect->mapOffsetRaw();
        path.moveTo(_points.begin()->position() - offset);
        for (auto it = (_points.begin() + 1); it < _points.end(); it++) {
            path.lineTo(it->position() - offset);
        }
        path.lineTo(_points.begin()->position() - offset);
    }
    p->drawPath(path);
}

template <typename T, typename W>
inline void PolyLineBase<T, W>::drawPoints(QPainter* p) const
{
    for (const W& point : _points) {
        point.draw(p, _rect);
    }
}

template <typename T, typename W>
inline const std::deque<W>& PolyLineBase<T, W>::points() const
{
    return _points;
}

template <typename T, typename W>
template <typename I>
void PolyLineBase<T, W>::drawArrowheadBetween(I it1, I it2, QPainterPath* path) const
{
    const QPointF& p1 = it1->position();
    const QPointF& p2 = it2->position();
    double angle = it1->angle();
    QPointF delta = p2 - p1;
    QPointF pa1 = p1 + delta * 0.2;
    drawArrowhead(pa1, angle, path);
    QPointF pa2 = p1 + delta * 0.8;
    drawArrowhead(pa2, angle, path);
}

template <typename T, typename W>
QPainterPath PolyLineBase<T, W>::createArrowheads(bool isConnected) const
{
    if (count() < 2) {
        return QPainterPath();
    }
    QPainterPath path;
    for (auto it = _points.begin(); it < (_points.end() - 1); it++) {
        drawArrowheadBetween(it, it + 1, &path);
    }
    if (isConnected && _points.size() > 2) {
        drawArrowheadBetween(_points.begin() + _points.size() - 1, _points.begin(), &path);
    }
    return path;
}

template <typename T, typename W>
void PolyLineBase<T, W>::drawArrowhead(const QPointF& pos, double angle, QPainterPath* path) const
{
    double arrowLen = 15;
    double angle1 = qDegreesToRadians(angle - 30);
    QPointF pos1 = pos - arrowLen * QPointF(std::cos(angle1), -std::sin(angle1));
    path->moveTo(pos1);
    path->lineTo(pos);
    double angle2 = qDegreesToRadians(angle + 30);
    QPointF pos2 = pos - arrowLen * QPointF(std::cos(angle2), -std::sin(angle2));
    path->lineTo(pos2);
}

template <typename T, typename W>
core::GeoBox PolyLineBase<T, W>::geoBox() const
{
    if (_points.size() == 0) {
        return core::GeoBox();
    }
    if (_points.size() == 1) {
        return core::GeoBox(_points[0].toLatLon(_rect));
    }

    QPointF topLeft = _points[0].position();
    QPointF bottomRight = topLeft;
    for (auto it = (_points.begin() + 1); it < _points.end(); it++) {
        double y = it->position().y();
        if (y > topLeft.y()) {
            topLeft.setY(y);
        } else if (y < bottomRight.y()) {
            bottomRight.setY(y);
        }
        double x = it->position().x();
        if (x < topLeft.x())
            topLeft.setX(x);
        else if (x > bottomRight.x())
            bottomRight.setX(x);
    }
    return core::GeoBox(WithPosition<>::toLatLon(topLeft, _rect), WithPosition<>::toLatLon(bottomRight, _rect));
}
}
}
}
