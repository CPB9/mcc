/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "mcc/ui/map/layers/RulerLayer.h"

#include <QMenu>

#include <cmath>

namespace mcc {
namespace ui {
namespace map {

static int pointRadius = 6;

static QPixmap createPixmap(const QColor& fillColor)
{
    int width = pointRadius * 2;
    QImage img(width, width, QImage::Format_ARGB32_Premultiplied);
    img.fill(Qt::transparent);
    QPainter p;
    p.begin(&img);
    p.setRenderHint(QPainter::Antialiasing);
    p.setBrush(fillColor);
    QPen pen;
    pen.setWidth(2);
    pen.setColor(Qt::black);
    p.setPen(pen);
    p.drawEllipse(0, 0, width, width);
    p.end();
    return QPixmap::fromImage(std::move(img));
}

RulerLayer::RulerLayer(const MapRectConstPtr& rect)
    : SimpleFlagLayer(rect)
    , _points(rect)
{
    _points.setLineWidth(3);
    _points.setLineColor(Qt::black);
    _activePixmap = createPixmap(Qt::yellow);
    _inactivePixmap = createPixmap(Qt::white);
    _contextMenu = new QMenu(rect->parent());
    //_addAction = _contextMenu->addAction("Добавить точку");
    _addAction = new QAction(mapRect()->parent());
    _removeAction = _contextMenu->addAction("Удалить точку");
    _copyAction = _contextMenu->addAction("Скопировать координаты");
    _contextMenu->addSeparator();
    _clearAction = _contextMenu->addAction("Очистить линейку");

    //     connect(_addAction, &QAction::triggered, this, [this]() {
    //         addFlagAt(mapRect()->fromWindowSystemCoordinates(_contextMenu->pos()));
    //         emit sceneUpdated();
    //     });
    connect(_removeAction, &QAction::triggered, this, [this]() {
        removeActive();
        emit sceneUpdated();
    });
    connect(_clearAction, &QAction::triggered, this, [this]() {
        _points.clear();
        _activeFlag = misc::None;
        _activeLine = misc::None;
        emit sceneUpdated();
    });
    connect(_copyAction, &QAction::triggered, this, [this]() {
        _points.at(_activeFlag.unwrap()).printCoordinatesToClipboard(mapRect());
        emit sceneUpdated();
    });
}

RulerLayer::~RulerLayer()
{
}

void RulerLayer::draw(QPainter* p) const
{
    _points.draw(p);

    if (_activeFlag.isSome() || _activeLine.isSome()) {
        double distance;
        QPointF pos;
        QPointF offset = _activePixmap.rect().center();
        if (_activeFlag.isSome()) {
            if (_activeFlag.unwrap() == (_points.count() - 1)) {
                distance = _points.totalDistance();
            } else {
                distance = _points.distanceTo(_activeFlag.unwrap());
            }
            pos = _points.at(_activeFlag.unwrap()).position();
        } else {
            pos = _activeLine.unwrap().pos;
            distance = _points.distanceTo(_activeLine.unwrap().index, pos);
            p->drawPixmap(pos - offset - mapRect()->mapOffsetRaw(), _activePixmap);
        }
        const char* suffix = "м";
        if (distance > 1000) {
            distance /= 1000;
            suffix = "км";
        }
        QString dtext = (QString("%1") + suffix).arg(distance, 0, 'f', 2) + " (%1, %2)";
        drawCoordinatesAt(p, pos, pos + QPointF(offset.x(), -offset.y()), dtext);
    }
    if (_points.count() > 1) {
        QPointF pos = _points.at(_points.count() - 1).position();
        double distance = _points.totalDistance();
        QPointF offset = _activePixmap.rect().center();
        const char* suffix = "м";
        if (distance > 1000) {
            distance /= 1000;
            suffix = "км";
        }
        QString dtext = (QString("%1") + suffix).arg(distance, 0, 'f', 2) + " (%1, %2)";
        drawCoordinatesAt(p, pos, pos + QPointF(offset.x(), -offset.y()), dtext);
    }
}

bool RulerLayer::zoomEvent(const QPoint& pos, int fromZoom, int toZoom)
{
    (void)pos;
    _points.changeZoomLevel(fromZoom, toZoom);
    return true;
}

void RulerLayer::changeProjection(const MercatorProjection& from, const MercatorProjection& to)
{
    _points.changeProjection(from, to);
}

bool RulerLayer::viewportResetEvent(int oldZoom, int newZoom, const QRect& oldViewpiort, const QRect& newViewport)
{
    (void)oldViewpiort;
    (void)newViewport;
    _points.changeZoomLevel(oldZoom, newZoom);
    return true;
}

void RulerLayer::addFlagAt(const QPointF& pos)
{
    insertFlagAt(_points.count(), pos);
}

void RulerLayer::insertFlagAt(std::size_t index, const QPointF& pos)
{
    BiMarker m(pos);
    m.setActivePixmap(_activePixmap);
    m.setInactivePixmap(_inactivePixmap);
    m.setActive(true);
    _points.emplace(index, std::move(m));
    _activeFlag = _points.count() - 1;
}

misc::Option<std::size_t> RulerLayer::flagAt(const QPointF& pos)
{
    return _points.nearest(pos);
}

misc::Option<LineIndexAndPos> RulerLayer::lineAt(const QPointF& pos)
{
    return _points.nearestLine(pos, pointRadius);
}

void RulerLayer::moveCurrentBy(const QPointF& delta)
{
    std::size_t index = _activeFlag.unwrap();
    _points.moveBy(index, delta);
}

void RulerLayer::removeActive()
{
    if (_activeFlag.isSome()) {
        std::size_t index = _activeFlag.unwrap();
        _points.remove(index);
        _activeFlag = misc::None;
    }
}

void RulerLayer::setCurrentActive(bool isActive)
{
    _points.at(_activeFlag.unwrap()).setActive(isActive);
}

void RulerLayer::execContextMenu(const QPoint& pos, bool hasActive)
{
    _addAction->setEnabled(!hasActive);
    _copyAction->setEnabled(hasActive);
    _removeAction->setEnabled(hasActive);
    _contextMenu->exec(mapRect()->toWindowSystemCoordinates(pos));
}

bool RulerLayer::showContextMenuForCurrent(const QPoint& pos)
{
    execContextMenu(pos, true);
    return true;
}

bool RulerLayer::showContextMenuForNone(const QPoint& pos)
{
    execContextMenu(pos, false);
    return true;
}

void RulerLayer::showFlagEditor(const QPoint& pos)
{
    (void)pos;
}

void RulerLayer::finishMovingCurrent()
{
}

misc::Option<core::GeoBox> RulerLayer::bbox() const
{
    if (_points.count() > 1) {
        return _points.geoBox();
    }
    return misc::None;
}
}
}
}
