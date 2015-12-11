/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "mcc/ui/map/layers/KmlPolygonLayer.h"
#include "mcc/ui/map/KmlUtils.h"

namespace mcc {
namespace ui {
namespace map {

KmlPolygonLayer::KmlPolygonLayer(const QString& label, const kmldom::CoordinatesPtr& coordinates,
                                 const kmldom::StylePtr& style, const MapRectConstPtr& rect)
    : KmlPolyLineLayer(label, coordinates, style, rect)
{
    QColor fillCOlor = Qt::white;
    if (style) {
        if (style->has_polystyle()) {
            kmldom::PolyStylePtr polyStyle = style->get_polystyle();
            if (polyStyle->get_fill()) {
                if (polyStyle->has_color()) {
                    fillCOlor = KmlUtils::qcolorFromKmlColor(polyStyle->get_color());
                }
            } else {
                fillCOlor = Qt::transparent;
            }
            if (!polyStyle->get_outline()) {
                _polyline.setLineColor(Qt::transparent);
            }
        }
    }
    _polyline.setFillColor(fillCOlor);
}

KmlPolygonLayer::~KmlPolygonLayer()
{
}

void KmlPolygonLayer::addFlagAt(const QPointF& pos)
{
    if (_polyline.count() < 2) {
        KmlPolyLineLayer::addFlagAt(pos);
    } else if (_polyline.count() == 2) {
        KmlPolyLineLayer::addFlagAt(pos);
        KmlPolyLineLayer::addFlagAt(_polyline.at(0).position());
    } else {
        KmlPolyLineLayer::insertFlagAt(_polyline.count() - 2, pos);
    }
}

void KmlPolygonLayer::insertFlagAt(std::size_t index, const QPointF& pos)
{
    if (_polyline.count() < 2) {
        KmlPolyLineLayer::insertFlagAt(index, pos);
    } else if (_polyline.count() == 2) {
        KmlPolyLineLayer::insertFlagAt(index, pos);
        KmlPolyLineLayer::addFlagAt(_polyline.at(0).position());
    } else {
        KmlPolyLineLayer::insertFlagAt(index, pos);
    }
}

void KmlPolygonLayer::removeActive()
{
    if (_polyline.count() == 4) {
        KmlPolyLineLayer::removeActive();
        _activeFlag = _polyline.count() - 1;
        KmlPolyLineLayer::removeActive();
    } else {
        KmlPolyLineLayer::removeActive();
    }
}

misc::Option<std::size_t> KmlPolygonLayer::flagAt(const QPointF& pos)
{
    auto index = _polyline.nearest(pos);
    if (index.isSome()) {
        if (index.unwrap() == (_polyline.count() - 1)) {
            return misc::None;
        }
    }
    return index;
}

void KmlPolygonLayer::draw(QPainter* p) const
{
    _polyline.drawPolygon(p);
    if (_isActive) {
        auto end = _polyline.points().end();
        if (_polyline.count() > 2) {
            end--;
        }
        for (auto it = _polyline.points().begin(); it < end; it++) {
            it->draw(p, mapRect());
        }
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
        p->drawPixmap(_activeLine.unwrap().pos - offset - mapRect()->mapOffsetRaw(), _activePixmap);
        drawCoordinatesAt(p, pos, pos + QPointF(offset.x(), -offset.y()));
    }
}

bool KmlPolygonLayer::hasElementAt(const QPointF& pos) const
{
    return _polyline.hasPointInside(pos);
}

void KmlPolygonLayer::moveCurrentBy(const QPointF& delta)
{
    std::size_t index = _activeFlag.unwrap();
    _polyline.moveBy(index, delta);
    core::LatLon latLon = _polyline.at(index).toLatLon(mapRect());
    _coordinates->set_coordinates_array_at(kmlbase::Vec3(latLon.longitude, latLon.latitude), index);
    if (index == 0) {
        index = _polyline.count() - 1;
        _polyline.moveBy(index, delta);
        core::LatLon latLon = _polyline.at(index).toLatLon(mapRect());
        _coordinates->set_coordinates_array_at(kmlbase::Vec3(latLon.longitude, latLon.latitude), index);
    }
}

KmlElementLayer::Type KmlPolygonLayer::type() const
{
    return KmlElementLayer::Polygon;
}
}
}
}
