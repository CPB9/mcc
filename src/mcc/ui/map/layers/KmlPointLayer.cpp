/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "mcc/ui/map/layers/KmlPointLayer.h"
#include "mcc/ui/map/KmlUtils.h"

#include <QPainter>
#include <QDebug>
#include <QMenu>

namespace mcc {
namespace ui {
namespace map {

static std::pair<QColor, double> createLabelParams(const kmldom::StylePtr& style)
{
    double labelScale = 1;
    QColor labelColor = Qt::white;
    if (style) {
        if (style->has_labelstyle()) {
            auto labelStyle = style->get_labelstyle();
            if (labelStyle->has_color()) {
                labelColor = KmlUtils::qcolorFromKmlColor(labelStyle->get_color());
            }
            if (labelStyle->has_scale()) {
                labelScale = labelStyle->get_scale();
            }
        }
    }
    return std::make_pair(labelColor, labelScale);
}

static QPixmap createPixmap(const kmldom::StylePtr& style, bool isActive)
{
    double iconScale = 1;
    QColor iconColor = Qt::white;
    QColor activeIconColor = Qt::black;
    if (style) {
        if (style->has_iconstyle()) {
            auto iconStyle = style->get_iconstyle();
            if (iconStyle->has_color()) {
                iconColor = KmlUtils::qcolorFromKmlColor(iconStyle->get_color());
                activeIconColor = KmlUtils::qcolorFromKmlColorInverted(iconStyle->get_color());
            }
            if (iconStyle->has_scale()) {
                iconScale = iconStyle->get_scale();
            }
        }
    }
    QColor color;
    if (isActive) {
        color = activeIconColor;
    } else {
        color = iconColor;
    }
    QSizeF size = QSizeF(15, 25) * iconScale;
    QImage img = Flag::drawFlag(size.width(), size.height(), color);
    return QPixmap::fromImage(std::move(img));
}

KmlPointLayer::KmlPointLayer(const QString& label, const kmldom::CoordinatesPtr& coordinates,
                             const kmldom::StylePtr& style, const MapRectConstPtr& rect)
    : KmlElementLayer(coordinates, style, rect)
    , _flag(rect)
    , _label(label)
{
    kmlbase::Vec3 vec3 = coordinates->get_coordinates_array_at(0);
    core::LatLon latLon;
    latLon.latitude = vec3.get_latitude();
    latLon.longitude = vec3.get_longitude();
    _flag.setLatLon(latLon, mapRect());
    _flag.setActivePixmap(createPixmap(style, true));
    _flag.setInactivePixmap(createPixmap(style, false));
    _flag.setLabel(label.toUtf8().constData());
    std::pair<QColor, double> labelParams = createLabelParams(style);
    _flag.setLabelBackground(labelParams.first);
    _flag.setLabelScale(labelParams.second);

    _contextMenu = new QMenu(mapRect()->parent());
    QAction* copyCoordinates = _contextMenu->addAction("Скопировать координаты");
    connect(copyCoordinates, &QAction::triggered, this, [this]() {
        _flag.printCoordinatesToClipboard(mapRect());
        emit sceneUpdated();
    });
}

KmlPointLayer::~KmlPointLayer()
{
}

bool KmlPointLayer::canAddPoints() const
{
    return false;
}

KmlElementLayer::Type KmlPointLayer::type() const
{
    return KmlElementLayer::Point;
}

void KmlPointLayer::setActive(bool isActive)
{
    if (mapRect()->cursorPosition().isSome()) {
        if (_flag.hasInRect(mapRect()->cursorPosition().unwrap())) {
            setCurrentActive(isActive);
            _activeFlag = 0;
        }
    }
    _isActive = isActive;
    if (isActive) {
        _flag.setLabelBackground(Qt::yellow);
    } else {
        _flag.setLabelBackground(createLabelParams(_style).first);
    }
}

void KmlPointLayer::draw(QPainter* p) const
{
    if (_activeFlag.isSome()) {
        _flag.drawWithoutLabel(p, mapRect());
        QPen pen;
        pen.setColor(Qt::black);
        p->setPen(pen);
        drawCoordinatesAt(p, QPointF(_flag.rect().center().x(), _flag.rect().bottom()), _flag.rect().topRight(),
                          _label + " (%1, %2)", _flag.labelScale());
    } else {
        _flag.draw(p, mapRect());
    }
}

bool KmlPointLayer::hasElementAt(const QPointF& pos) const
{
    return _flag.hasInRect(pos);
}

bool KmlPointLayer::zoomEvent(const QPoint& pos, int fromZoom, int toZoom)
{
    (void)pos;
    _flag.changeZoomLevel(fromZoom, toZoom);
    return true;
}

void KmlPointLayer::changeProjection(const MercatorProjection& from, const MercatorProjection& to)
{
    _flag.changeProjection(mapRect(), from, to);
}

bool KmlPointLayer::viewportResetEvent(int oldZoom, int newZoom, const QRect& oldViewpiort, const QRect& newViewport)
{
    (void)oldViewpiort;
    (void)newViewport;
    _flag.changeZoomLevel(oldZoom, newZoom);
    return true;
}

void KmlPointLayer::updateKml()
{
}

void KmlPointLayer::addFlagAt(const QPointF& pos)
{
    (void)pos;
}

void KmlPointLayer::insertFlagAt(std::size_t index, const QPointF& pos)
{
    (void)index;
    (void)pos;
}

misc::Option<std::size_t> KmlPointLayer::flagAt(const QPointF& pos)
{
    if (_flag.rect().contains(pos)) {
        return std::size_t(0);
    }
    return misc::None;
}

misc::Option<LineIndexAndPos> KmlPointLayer::lineAt(const QPointF& pos)
{
    (void)pos;
    return misc::None;
}

void KmlPointLayer::moveCurrentBy(const QPointF& delta)
{
    _flag.moveBy(delta);
    core::LatLon latLon = _flag.toLatLon(mapRect());
    kmlbase::Vec3 vec3(latLon.longitude, latLon.latitude);
    _coordinates->set_coordinates_array_at(vec3, 0);
}

void KmlPointLayer::setCurrentActive(bool isActive)
{
    _flag.setActive(isActive);
}

bool KmlPointLayer::showContextMenuForCurrent(const QPoint& pos)
{
    _contextMenu->exec(mapRect()->toWindowSystemCoordinates(pos));
    (void)pos;
    return false;
}

bool KmlPointLayer::showContextMenuForNone(const QPoint& pos)
{
    (void)pos;
    return false;
}

void KmlPointLayer::showFlagEditor(const QPoint& pos)
{
    (void)pos;
}

void KmlPointLayer::finishMovingCurrent()
{
}
}
}
}
