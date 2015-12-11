/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "mcc/ui/map/layers/AircraftLayer.h"
#include "mcc/ui/map/layers/TailLayer.h"
#include "mcc/ui/core/FlyingDevice.h"

#include <QColor>
#include <QTimer>
#include <QDebug>

namespace mcc {
namespace ui {
namespace map {

using core::FlyingDevice;
using core::GeoOrientation;
using core::GeoPosition;
using core::TrailMode;

static int pixmapWidth = 40;

AircraftLayer::AircraftLayer(core::FlyingDevice* model, const MapRectConstPtr& rect)
    : Layer(rect)
    , _model(model)
    , _tail(new TailLayer(rect))
    , _dogTail(new TailLayer(rect))
    , _blinkTimer(new QTimer)
    , _dogPos(QPointF(0, 0))
    , _isActive(_model->isActive())
    , _positionValid(false)
    , _dogValid(false)
    , _isBlinkState(false)
{
    _tail->setLineColor(Qt::red);
    _tail->setTrailModeAndParam(_model->trailMode(), _model->trailCount());
    _dogTail->setLineColor(Qt::darkMagenta);
    _dogTail->setTrailModeAndParam(_model->trailMode(), _model->trailCount());
    _blinkTimer->setSingleShot(false);
    _blinkTimer->setInterval(500);

    connect(_blinkTimer.get(), &QTimer::timeout, this, [this]() {
        _isBlinkState = !_isBlinkState;
        repaintAircraftPixmap();
        emit sceneUpdated();
    });

    connect(_model, &FlyingDevice::accuracyChanged, this, &AircraftLayer::updateAccuracy);

    connect(_model, &FlyingDevice::orientationChanged, this, [this](const GeoOrientation& orientation) {
        _lastOrientation = orientation;
        if (_positionValid) {
            repaintAircraftPixmap();
            emit sceneUpdated();
        }
    });

    connect(_model, &FlyingDevice::signalBad, this, [this]() {
        _tail->setSignalLost();
        if (!_blinkTimer->isActive()) {
            _blinkTimer->start();
        }
    });

    connect(_model, &FlyingDevice::signalGood, this, [this]() {
        _isBlinkState = false;
        _blinkTimer->stop();
        repaintAircraftPixmap();
        emit sceneUpdated();
    });

    connect(_model, &FlyingDevice::trailModeChanged, this, [this](TrailMode mode) {
        _tail->setTrailModeAndParam(mode, _model->trailCount());
        _dogTail->setTrailModeAndParam(mode, _model->trailCount());
        emit sceneUpdated();
    });

    connect(_model, &FlyingDevice::trailCountChanged, this, [this](int count) {
        _tail->setTrailModeAndParam(_model->trailMode(), count);
        _dogTail->setTrailModeAndParam(_model->trailMode(), count);
        emit sceneUpdated();
    });

    connect(_model, &FlyingDevice::trailCleared , this, [this]() {
        _tail->clear();
        _dogTail->clear();
        emit sceneUpdated();
    });

    connect(_model, &FlyingDevice::dogPositionChanged, this, [this](const GeoPosition& position) {
        _dogValid = true;
        core::LatLon dogLatLon(position.latitude, position.longitude);
        _dogPos.setLatLon(dogLatLon, mapRect());
        _dogTail->addPoint(_model->dogPosition(), _model->lastTmMsgDateTime());
        emit sceneUpdated();
    });

    connect(_model, &FlyingDevice::positionChanged, this, [this](const GeoPosition& position) {
        _lastPosition = position;
        _positionValid = true;
        core::LatLon latLon(position.latitude, position.longitude);
        _aircraftMarker.setLatLon(latLon, mapRect());
        _tail->addPoint(position, _model->lastTmMsgDateTime());
        emit sceneUpdated();
    });

    connect(_model, &FlyingDevice::pixmapChanged, this, [this](const QPixmap& pixmap) {
        _aircraftImage = pixmap.toImage().scaled(pixmapWidth, pixmapWidth, Qt::KeepAspectRatio);
        repaintAircraftPixmap();
        emit sceneUpdated();
    });

    connect(_tail.get(), &TailLayer::sceneUpdated, this, &AircraftLayer::sceneUpdated);
    connect(_dogTail.get(), &TailLayer::sceneUpdated, this, &AircraftLayer::sceneUpdated);
    _aircraftImage
        = _model->pixmap().toImage().scaled(pixmapWidth, pixmapWidth, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    repaintAircraftPixmap();
    updateAccuracy();
    emit sceneUpdated();
}

AircraftLayer::~AircraftLayer()
{
}

void AircraftLayer::updateAccuracy()
{
    _accuracyInPixels = _model->accuracy() / mapRect()->projection().mapWidth() * mapRect()->maxMapSize();
}

void AircraftLayer::setActive(bool isActive)
{
    if (_isActive == isActive) {
        return;
    }
    _isActive = isActive;
    repaintAircraftPixmap();
}

void AircraftLayer::repaintAircraftPixmap()
{
    QTransform transform;
    transform.rotate(_lastOrientation.heading);
    QImage newImage = _aircraftImage;
    QPainter p(&newImage);
    if (_isActive) {
        p.setRenderHint(QPainter::Antialiasing);
        p.setBrush(Qt::NoBrush);
        QPen pen;
        pen.setWidth(2);
        pen.setColor(Qt::black);
        p.setPen(pen);
        p.drawEllipse(newImage.rect());
    }
    if (_isBlinkState) {
        QRect rect = newImage.rect();
        QPen pen;
        pen.setWidth(2);
        pen.setColor(Qt::red);
        pen.setStyle(Qt::DashLine);
        p.setPen(pen);
        p.drawLine(rect.topLeft(), rect.bottomRight());
        p.drawLine(rect.topRight(), rect.bottomLeft());
    }
    _aircraftMarker.setPixmap(QPixmap::fromImage(newImage.transformed(transform, Qt::SmoothTransformation)));
}

void AircraftLayer::draw(QPainter* p) const
{
    if (_positionValid) {
        const QPoint& offset =  mapRect()->mapOffsetRaw();
        if (_isActive && _dogValid) {
            _dogTail->draw(p);
            QPen pen;
            pen.setWidth(2);
            pen.setColor(Qt::magenta);
            p->setPen(pen);
            p->setBrush(Qt::yellow);
            p->drawLine(_aircraftMarker.position() - offset, _dogPos.position() - offset);
            p->drawEllipse(_dogPos.position() - offset, 5, 5);
        }
        _tail->draw(p);
        _aircraftMarker.draw(p, mapRect());
        if (_accuracyInPixels > pixmapWidth / 2) {
            QPen pen;
            pen.setWidthF(1);
            pen.setColor(Qt::darkGray);
            pen.setStyle(Qt::DashDotLine);
            p->setPen(pen);
            p->setBrush(Qt::NoBrush);
            p->drawEllipse(_aircraftMarker.rect().center() - offset, _accuracyInPixels, _accuracyInPixels);
        }
        if (mapRect()->cursorPosition().isSome()) {
            if (_aircraftMarker.hasInRect(mapRect()->cursorPosition().unwrap())) {
                drawLabel(p);
            }
        }
    }
}

void AircraftLayer::drawLabel(QPainter* p) const
{
    QString text = "%1\nЗаряд: %2 %\nСигнал: %3 %\nВысота: %4 м\nСкорость: %5 м/с";//Широта: %6\nДолгота: %7";
    text = text.arg(_model->deviceDescription()._device_info)
               .arg(_model->battery())
               .arg(_model->signal())
               .arg(_model->position().altitude, 0, 'f', 2)
               .arg(_model->speed(), 0, 'f', 2);
    Qt::Alignment alignment;
    QRectF rect = _aircraftMarker.rect();
    QSize size = LabelBase::computeSize(text);
    QPointF mapPos = mapRect()->mapOffset();
    QPoint pos;
    if ((rect.top() - size.height()) < mapPos.y()) {
        alignment |= Qt::AlignTop;
        pos.setY(rect.bottom());
    } else {
        alignment |= Qt::AlignBottom;
        pos.setY(rect.top());
    }
    if ((rect.right() + size.width()) > (mapPos.x() + mapRect()->size().width())) {
        alignment |= Qt::AlignRight;
        pos.setX(rect.left());
    } else {
        alignment |= Qt::AlignLeft;
        pos.setX(rect.right());
    }
    QPen pen;
    pen.setWidth(1);
    pen.setColor(Qt::black);
    p->setPen(pen);
    Label::drawLabelAt(p, text, pos - mapRect()->mapOffsetRaw(), QColor(255,255,179), alignment);
}

bool AircraftLayer::mouseMoveEvent(const QPoint& oldPos, const QPoint& newPos)
{
    (void)oldPos;
    if (_aircraftMarker.hasInRect(newPos)) {
        return true;
    }
    return false;
}

bool AircraftLayer::mousePressEvent(const QPoint& pos)
{
    (void)pos;
    return false;
}

bool AircraftLayer::mouseReleaseEvent(const QPoint& pos)
{
    (void)pos;
    return false;
}

bool AircraftLayer::viewportResizeEvent(const QSize& oldSize, const QSize& newSize)
{
    (void)oldSize;
    (void)newSize;
    return false;
}

bool AircraftLayer::viewportScrollEvent(const QPoint& oldPos, const QPoint& newPos)
{
    (void)oldPos;
    (void)newPos;
    return false;
}

bool AircraftLayer::contextMenuEvent(const QPoint& pos)
{
    (void)pos;
    return false;
}

bool AircraftLayer::zoomEvent(const QPoint& pos, int from, int to)
{
    _aircraftMarker.changeZoomLevel(from, to);
    _dogPos.changeZoomLevel(from, to);
    _tail->zoomEvent(pos, from, to);
    _dogTail->zoomEvent(pos, from, to);
    updateAccuracy();
    return true;
}

void AircraftLayer::changeProjection(const MercatorProjection& from, const MercatorProjection& to)
{
    _aircraftMarker.changeProjection(mapRect(), from, to);
    _dogPos.changeProjection(mapRect(), from, to);
    _tail->changeProjection(from, to);
    _dogTail->changeProjection(from, to);
    updateAccuracy();
}

bool AircraftLayer::viewportResetEvent(int from, int to, const QRect& oldViewpiort, const QRect& newViewport)
{
    _aircraftMarker.changeZoomLevel(from, to);
    _dogPos.changeZoomLevel(from, to);
    _tail->viewportResetEvent(from, to, oldViewpiort, newViewport);
    _dogTail->viewportResetEvent(from, to, oldViewpiort, newViewport);
    updateAccuracy();
    return true;
}

bool AircraftLayer::hasDeviceAt(const QPoint& pos)
{
    return _aircraftMarker.hasInRect(pos);
}
}
}
}
