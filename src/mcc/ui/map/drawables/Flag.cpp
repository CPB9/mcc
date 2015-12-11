/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "mcc/ui/map/drawables/Flag.h"
#include "mcc/ui/core/GlobalCoordinatePrinter.h"

#include <QSvgRenderer>

namespace mcc {
namespace ui {
namespace map {

QImage renderSvg(const QString& path, int width, int height)
{
    QSvgRenderer renderer(path);
    QImage img(width, height, QImage::Format_ARGB32);
    img.fill(0x00000000);
    QPainter p(&img);
    renderer.render(&p);

    return img;
}

QImage Flag::drawFlag(quint32 width, quint32 height, QColor color)
{
    QImage flagImg(width, height, QImage::Format_ARGB32);
    flagImg.fill(Qt::transparent);
    QPainter painter(&flagImg);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setRenderHint(QPainter::HighQualityAntialiasing);

    QRectF outerFlagRect(0., 0., width + 1, height + 1);
    double min(qMin(width, height));
    QPointF circleCenter((width + 1) / 2., min / 2.);

    painter.setPen(Qt::NoPen);
    painter.setBrush(Qt::black);
    drawFlag(painter, outerFlagRect);

    double innerCircleRadius(min / 2.);
    Q_UNUSED(innerCircleRadius);
    QRectF innerFlagRect(outerFlagRect.adjusted(1, 1, -1, -1));
    QLinearGradient colorGradient(innerFlagRect.topLeft(), innerFlagRect.bottomRight());
    colorGradient.setColorAt(0, color.lighter(150));
    colorGradient.setColorAt(1, color.darker(150));
    painter.setBrush(colorGradient);
    drawFlag(painter, innerFlagRect);

    double radius(min / 3.);
    QRectF innerCircleRect(circleCenter.x() - radius, circleCenter.y() - radius, radius * 2., radius * 2.);
    painter.setBrush(Qt::gray);
    painter.drawEllipse(innerCircleRect);

    innerCircleRect = innerCircleRect.adjusted(1, 1, -1, -1);
    QLinearGradient blackGradient(innerCircleRect.topLeft(), innerCircleRect.bottomRight());
    blackGradient.setColorAt(0, Qt::gray);
    blackGradient.setColorAt(1, Qt::white);
    painter.setBrush(blackGradient);
    painter.drawEllipse(innerCircleRect);

    return flagImg;
}

QRectF computeFlagRect(const QRectF& rect)
{
    double rectWidth(rect.width()), rectHeight(rect.height());
    return QRectF(rectWidth / 4., rectHeight / 4., rectWidth / 2., rectHeight * .75);
}

QPointF computeCircleCenterForFlagRect(const QRectF& rect)
{
    double flagMin(qMin(rect.width(), rect.height()));
    return QPointF(rect.left() + (rect.width() + 1) / 2., rect.top() + flagMin / 2.);
}

typedef std::function<void(QPainter&, const QRectF&)> WaypointFlagPainter;

WaypointFlagPainter waypointFlagPainters[] = {
    //! Дом
    [](QPainter& painter, const QRectF& rect) {
        QImage img = renderSvg(":/resources/waypoints/home.svg", rect.width() / 2., rect.height() / 2.);
        painter.drawImage(QPointF(rect.width() / 2., rect.height() / 2.), img);
    },
    //! Разворот
    [](QPainter& painter, const QRectF& rect) {
        double rectWidth(rect.width()), rectHeight(rect.height());
        double min(qMin(rectWidth, rectHeight));
        QPointF circleCenter(computeCircleCenterForFlagRect(computeFlagRect(rect)));
        double radius(min / 5.);
        QRectF innerCircleRect(circleCenter.x() - radius, circleCenter.y() - radius, radius * 2, radius * 2);
        painter.setPen(QPen(Qt::red, rectWidth / 14.));
        painter.drawArc(innerCircleRect, -45 * 16, 270 * 16);
        double angleRad(qDegreesToRadians(135.));
        const QPointF start(circleCenter.x() + radius * qCos(angleRad) + rectWidth / 64.,
                            circleCenter.y() + radius * qSin(angleRad) + rectWidth / 64.);
        double arrowAngleRad(qDegreesToRadians(180.));
        double arrowSecondAngleRad(qDegreesToRadians(180. + 90.));
        double arrowLength(radius / 3);
        painter.drawLine(start, start + QPointF(arrowLength * qCos(arrowAngleRad), arrowLength * qSin(arrowAngleRad)));
        painter.drawLine(
            start, start + QPointF(arrowLength * qCos(arrowSecondAngleRad), arrowLength * qSin(arrowSecondAngleRad)));
    },
    //! Под достижению высоты
    [](QPainter& painter, const QRectF& rect) {
        QImage img = renderSvg(":/resources/waypoints/by_height.svg", rect.width() / 2., rect.height() / 2.);
        painter.drawImage(QPointF(0, 0), img);
    },
    //! Цель
    [](QPainter& painter, const QRectF& rect) {
        QImage img = renderSvg(":/resources/waypoints/target1.svg", rect.width(), rect.height());
        painter.drawImage(rect, img);
    },
    //! Переключение
    [](QPainter& painter, const QRectF& rect) {
        QImage img = renderSvg(":/resources/waypoints/switch_route.svg", rect.width(), rect.height());
        painter.drawImage(rect, img);
    },
    //! Возврат на маршрут
    [](QPainter& painter, const QRectF& rect) {
        QImage img = renderSvg(":/resources/waypoints/return_to_route.svg", rect.width(), rect.height());
        painter.drawImage(rect, img);
    },
    //! Ожидание
    [](QPainter& painter, const QRectF& rect) {
        QImage img = renderSvg(":/resources/waypoints/pause.svg", rect.width() / 2., rect.height() / 2.);
        painter.drawImage(QPointF(rect.width() / 2., 0), img);
    },
    //! Интерполяция
    [](QPainter& painter, const QRectF& rect) {
        QImage img = renderSvg(":/resources/waypoints/interpolate.svg", rect.width(), rect.height());
        painter.drawImage(rect, img);
    },
    //! Управление по курсу
    [](QPainter& painter, const QRectF& rect) {
        QImage img = renderSvg(":/resources/waypoints/by_heading.svg", rect.width() / 2., rect.height() / 2.);
        painter.drawImage(QPointF(0, rect.height() / 2.), img);
    }};

QImage Flag::drawWaypointFlag(quint32 width, quint32 height, QColor color, quint32 waypointNumber,
                              quint32 waypointFlags)
{
    QImage waypointImage(width, height, QImage::Format_ARGB32);
    waypointImage.fill(Qt::transparent);
    QImage flagImage(drawWaypointFlag(width, height, color));
    QPainter painter(&waypointImage);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setRenderHint(QPainter::HighQualityAntialiasing);
    painter.drawImage(QPointF(0., 0.), flagImage);
    QPointF circleCenter(computeCircleCenterForFlagRect(computeFlagRect(QRectF(0., 0., width, height))));
    double min(qMin(width, height));
    painter.setPen(Qt::black);
    painter.drawText(QRectF(circleCenter.x() - min / 2., circleCenter.y() - min / 2., min, min * .98), Qt::AlignCenter,
                     QString::number(waypointNumber));
    if (waypointFlags != 0) {
        QRectF rect(0, 0, width, height);
        for (quint32 i(0); i < sizeof(waypointFlagPainters) / sizeof(waypointFlagPainters[0]); i++) {
            if ((waypointFlags & (1 << i)) != 0) {
                waypointFlagPainters[i](painter, rect);
            }
        }
    }
    return waypointImage;
}

QImage Flag::drawWaypointFlag(quint32 width, quint32 height, QColor color)
{
    QImage waypointImage(width, height, QImage::Format_ARGB32);
    waypointImage.fill(Qt::transparent);
    QImage flagImage(drawFlag(width / 2., height * 0.75, color));
    QPainter painter(&waypointImage);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setRenderHint(QPainter::HighQualityAntialiasing);
    QRectF flagRect(computeFlagRect(QRect(0., 0., width, height)));
    painter.drawImage(flagRect.topLeft(), flagImage);
    return waypointImage;
}

void Flag::drawFlag(QPainter& painter, const QRectF& rect)
{
    double min(qMin(rect.width(), rect.height()));
    double centerX(rect.center().x()); //, centerY(rect.center().y());
    double radius(min / 2. - 1.);
    QRectF circleRect(centerX - radius, rect.top() + 1, radius * 2., radius * 2.);
    painter.drawEllipse(circleRect);
    QPolygonF poly;
    double angleFromRad(qDegreesToRadians(30.)), angleToRad(qDegreesToRadians(150.));
    poly << QPointF(centerX, rect.bottom())
         << QPointF(centerX + radius * qCos(angleFromRad), circleRect.center().y() + radius * qSin(angleFromRad))
         << QPointF(centerX + radius * qCos(angleToRad), circleRect.center().y() + radius * qSin(angleToRad));
    poly << poly.at(0);
    painter.drawPolygon(poly);
}

Flag::Flag(const QPointF& position, const QPixmap& active, const QPixmap& inactive, const QString& name,
           const MapRectConstPtr& rect)
    : _name(name)
    , _point(position)
    , _rect(rect)
{
    _isActive = false;
    _label.setLabel(name);
    _label.setLabelAlignment(Qt::AlignBottom | Qt::AlignLeft);
    _activeMarker.setPixmap(active);
    _activeMarker.setAlignment(Qt::AlignBottom);
    _inactiveMarker.setPixmap(inactive);
    _inactiveMarker.setAlignment(Qt::AlignBottom);
}

Flag::Flag(const MapRectConstPtr& rect)
    : Flag(QPointF(0, 0), QPixmap(), QPixmap(), QString(), rect)
{
}

void Flag::setActive(bool isActive)
{
    _isActive = isActive;
    updateLabel();
}

void Flag::setLabelBackground(const QColor& color)
{
    _label.setLabelBackground(color);
}

void Flag::setLabelScale(double scale)
{
    _label.setLabelScale(scale);
}

void Flag::updateLabel()
{
    if (_isActive) {
        const core::CoordinatePrinter& printer(core::GlobalCoordinatePrinter::instance()->printer());
        core::LatLon latLon(toLatLon(_point.position(), _rect));
        double lat(latLon.latitude), lon(latLon.longitude);
        _label.setLabel(_name + QString(" (%1 %2)").arg(printer.printLat(lat, lon), printer.printLon(lat, lon)));
    } else {
        _label.setLabel(_name);
    }
}

bool Flag::isActive() const
{
    return _isActive;
}

void Flag::changeZoomLevel(int from, int to)
{
    _point.changeZoomLevel(from, to);
}

void Flag::draw(QPainter* p, const MapRectConstPtr& rect) const
{
    drawWithoutLabel(p, rect);
    drawLabel(p, rect);
}

void Flag::drawWithoutLabel(QPainter* p, const MapRectConstPtr& rect) const
{
    if (_isActive) {
        _activeMarker.drawMarker(p, _point.position() - rect->mapOffsetRaw());
    } else {
        _inactiveMarker.drawMarker(p, _point.position() - rect->mapOffsetRaw());
    }
}

void Flag::drawLabel(QPainter* p, const MapRectConstPtr& rect) const
{
    _label.drawAt(p, this->rect().topRight() - rect->mapOffsetRaw());
}

void Flag::setActivePixmap(const QPixmap& p)
{
    _activeMarker.setPixmap(p);
}

void Flag::setInactivePixmap(const QPixmap& p)
{
    _inactiveMarker.setPixmap(p);
}

void Flag::setLabel(const QString& label)
{
    _name = label;
    updateLabel();
}

void Flag::moveBy(const QPointF& delta)
{
    _point.moveBy(delta);
    updateLabel();
}

QRectF Flag::rect() const
{
    if (isActive()) {
        return WithRect::positionedRect(_activeMarker.rect(), _point.position() + _activeMarker.offset());
    }
    return WithRect::positionedRect(_inactiveMarker.rect(), _point.position() + _inactiveMarker.offset());
}

void Flag::setName(const QString& name)
{
    _name = name;
    updateLabel();
}

const QString& Flag::name() const
{
    return _name;
}

void Flag::setPosition(const QPointF& position)
{
    _point.setPosition(position);
    updateLabel();
}
}
}
}
