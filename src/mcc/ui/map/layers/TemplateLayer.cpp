/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "mcc/ui/map/layers/TemplateLayer.h"
#include "mcc/ui/map/layers/WaypointLayer.h"
#include "mcc/ui/core/Route.h"
#include "mcc/ui/core/FlyingDevice.h"

#include <QDebug>

namespace mcc {
namespace ui {
namespace map {

static double pixmapWidth = 40;
static double radius = pixmapWidth * 0.7;

TemplateLayer::TemplateLayer(const MapRectConstPtr& rect)
    : Layer(rect)
    , _state(None)
    , _rotation(0)
    , _oldRotation(0)
{
    QRectF mapRect = rect->visibleMapRect();
    _width = mapRect.width() * 0.6;
    _height = mapRect.height() * 0.6;
    _center = mapRect.center() - rect->mapOffsetRaw();
    _rotatePixmap
        = QPixmap::fromImage(QImage(":resources/rotate.png")
                                 .scaled(pixmapWidth, pixmapWidth, Qt::IgnoreAspectRatio, Qt::SmoothTransformation)
                                 .mirrored());
    _expandPixmap
        = QPixmap::fromImage(QImage(":resources/expand.png")
                                 .scaled(pixmapWidth, pixmapWidth, Qt::IgnoreAspectRatio, Qt::SmoothTransformation)
                                 .mirrored());
    //   _box.setLineColor(QColor(128, 177, 211));
    //   _box.setFillColor(QColor(190, 186, 218, 100));
    //   _box.setLineWidth(2);
}

TemplateLayer::~TemplateLayer()
{
}

void TemplateLayer::rotate(QPointF* point, double angle) const
{
    double a = qDegreesToRadians(angle);
    double ca = std::cos(a);
    double sa = std::sin(a);
    double x = point->x() * ca - point->y() * sa;
    double y = point->x() * sa + point->y() * ca;
    point->setX(x);
    point->setY(y);
}

QPointF TemplateLayer::rotated(const QPointF& point) const
{
    QPointF p = point;
    rotate(&p, -_rotation);
    return p;
}

void TemplateLayer::changeProjection(const MercatorProjection& from, const MercatorProjection& to)
{
    (void)from;
    (void)to;
    // reset();
}

bool TemplateLayer::contextMenuEvent(const QPoint& pos)
{
    (void)pos;
    return false;
}

static void drawArrowhead(const QPointF& pos, double angle, QPainter* p)
{
    double arrowLen = 15;
    double angle1 = qDegreesToRadians(angle - 30);
    QPointF pos1 = pos - arrowLen * QPointF(std::cos(angle1), -std::sin(angle1));
    p->drawLine(pos, pos1);
    double angle2 = qDegreesToRadians(angle + 30);
    QPointF pos2 = pos - arrowLen * QPointF(std::cos(angle2), -std::sin(angle2));
    p->drawLine(pos, pos2);
}

static void drawArrowheads(const QPointF& p1, const QPointF& p2, QPainter* p)
{
    QPointF delta = p2 - p1;
    double angle = qRadiansToDegrees(std::atan2(-p2.y() + p1.y(), p2.x() - p1.x()));
    drawArrowhead(p1 + 0.25 * delta, angle, p);
    drawArrowhead(p1 + 0.75 * delta, angle, p);
}

void TemplateLayer::draw(QPainter* p) const
{
    QTransform t = p->transform();
    p->translate(_center);
    p->rotate(-_rotation);
    double dx = _width / 2;
    double dy = _height / 2;
    std::array<QPointF, 4> points = {{QPointF(-dx, -dy), QPointF(dx, -dy), QPointF(dx, dy), QPointF(-dx, dy)}};
    p->setBrush(QColor(190, 186, 218, 100));
    QPen pen;
    pen.setWidth(2);
    pen.setColor(QColor(128, 177, 211));
    pen.setStyle(Qt::DashLine);
    p->setPen(pen);
    p->drawPolygon(points.data(), 4);
    //     p->setBrush(QColor(141,211,199, 100));
    //     pen.setColor(QColor(251,128,114));
    //     p->setPen(pen);
    double pw2 = pixmapWidth / 2;
    double r = radius;
    p->drawEllipse(points[2], r, r);
    p->drawEllipse(points[3], r, r);
    p->drawPixmap(points[2] + QPointF(-pw2, -pw2), _expandPixmap);
    p->drawPixmap(points[3] + QPointF(-pw2, -pw2), _rotatePixmap);
    pen.setColor(Qt::black);
    p->setPen(pen);
    p->drawLine(0, -radius, 0, radius);
    p->drawLine(-radius, 0, radius, 0);
    pen.setStyle(Qt::SolidLine);
    p->setPen(pen);
    drawArrowheads(points[0], points[1], p);
    drawArrowheads(points[3], points[0], p);
    p->setTransform(t);
}

bool TemplateLayer::nearby(const QPointF& first, const QPointF& second) const
{
    QPointF delta = second - first;
    return std::hypot(delta.x(), delta.y()) < radius;
}

TemplateLayer::State TemplateLayer::computeState(const QPoint& pos) const
{
    QPointF vec = pos - mapRect()->mapOffsetRaw() - _center;
    rotate(&vec, _rotation);
    double dx = _width / 2;
    double dy = _height / 2;
    if (nearby(vec, QPointF(dx, dy))) {
        return Scaling;
    }
    if (nearby(vec, QPointF(-dx, dy))) {
        return Rotating;
    }
    if (std::abs(vec.x()) <= std::abs(_width / 2) && std::abs(vec.y()) <= std::abs(_height / 2)) {
        return Moving;
    }
    return None;
}

bool TemplateLayer::updateCursor(TemplateLayer::State state)
{
    if (state == Moving) {
        setCursor(Qt::OpenHandCursor);
        return true;
    } else if (state == Scaling) {
        setCursor(Qt::SizeAllCursor);
        return true;
    } else if (state == Rotating) {
        setCursor(Qt::PointingHandCursor);
        return true;
    } else {
        setCursor(Qt::ArrowCursor);
    }
    return false;
}

bool TemplateLayer::mouseMoveEvent(const QPoint& oldPos, const QPoint& newPos)
{
    if (_state == Moving) {
        QPointF delta = newPos - oldPos;
        _center += delta;
        return true;
    } else if (_state == Scaling) {
        QPointF vec = newPos - mapRect()->mapOffsetRaw() - _pressedPos;
        rotate(&vec, _rotation);
        _width = _oldWidth + vec.x() * 2;
        _height = _oldHeight + vec.y() * 2;
    } else if (_state == Rotating) {
        QPointF pos = newPos - mapRect()->mapOffsetRaw();
        double a1 = WithPosition<>::angleBetween(_center, pos);
        double a2 = WithPosition<>::angleBetween(_center, _pressedPos);
        _rotation = _oldRotation + a1 - a2;
    }
    State newState = computeState(newPos);
    return updateCursor(newState);
}

bool TemplateLayer::mousePressEvent(const QPoint& pos)
{
    _pressedPos = pos - mapRect()->mapOffsetRaw();
    _oldRotation = _rotation;
    _oldHeight = _height;
    _oldWidth = _width;
    _state = computeState(pos);
    return updateCursor(_state);
}

bool TemplateLayer::mouseReleaseEvent(const QPoint& pos)
{
    (void)pos;
    _state = None;
    return false;
}

bool TemplateLayer::viewportResetEvent(int oldZoom, int newZoom, const QRect& oldViewpiort, const QRect& newViewport)
{
    (void)oldZoom;
    (void)newZoom;
    (void)oldViewpiort;
    (void)newViewport;
    return false;
}

bool TemplateLayer::viewportResizeEvent(const QSize& oldSize, const QSize& newSize)
{
    (void)oldSize;
    (void)newSize;
    return false;
}

bool TemplateLayer::viewportScrollEvent(const QPoint& oldPos, const QPoint& newPos)
{
    (void)oldPos;
    (void)newPos;
    return false;
}

bool TemplateLayer::zoomEvent(const QPoint& pos, int fromZoom, int toZoom)
{
    (void)pos;
    (void)fromZoom;
    (void)toZoom;
    return false;
}

std::vector<core::LatLon> TemplateLayer::createScanLine(const QPointF& p1, const QPointF& p2, double distance) const
{
    core::LatLon start = mapRect()->latLon(p1.toPoint());
    core::LatLon end = mapRect()->latLon(p2.toPoint());
    double totalDistance = mapRect()->projection().calcDistance(start, end);
    int numPoints = std::trunc(totalDistance / distance + 1);
    if (numPoints < 2) {
        numPoints = 2;
    }
    if (numPoints > 25) {
        numPoints = 25; //HACK
    }
    core::LatLon delta((end.latitude - start.latitude) / numPoints, (end.longitude - start.longitude) / numPoints);
    std::vector<core::LatLon> points;
    for (int i = 0; i < numPoints; i++) {
        points.push_back(start);
        start.latitude += delta.latitude;
        start.longitude += delta.longitude;
    }
    return points;
}

WaypointLayer* TemplateLayer::intoWaypointLayer(core::FlyingDevice* device, core::Route* route, double distance, double height, double speed) const
{
    route->clear();
    route->setRing(false);
    QPointF topLeft = rotated(QPointF(-_width / 2, -_height / 2)) + _center;
    QPointF topRight = rotated(QPointF(_width / 2, -_height / 2)) + _center;
    QPointF bottomLeft = rotated(QPointF(-_width / 2, _height / 2)) + _center;
    QPointF bottomRight = rotated(QPointF(_width / 2, _height / 2)) + _center;
    std::vector<core::LatLon> topLine = createScanLine(topLeft, topRight, distance);
    std::vector<core::LatLon> bottomLine = createScanLine(bottomLeft, bottomRight, distance);
    std::size_t size = std::min(topLine.size(), bottomLine.size());
    std::size_t i = 0;
    while (i < size) {
        route->addWaypointNoEmit(core::Waypoint(bottomLine[i], height, speed));
        route->addWaypointNoEmit(core::Waypoint(topLine[i], height, speed));
        i++;
        if (i >= size) {
            break;
        }
        route->addWaypointNoEmit(core::Waypoint(topLine[i], height, speed));
        route->addWaypointNoEmit(core::Waypoint(bottomLine[i], height, speed));
        i++;
    }
    emit route->routeChanged();
    return new WaypointLayer(device, route, mapRect());
}
}
}
}
