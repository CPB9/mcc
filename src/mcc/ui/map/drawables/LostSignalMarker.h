/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#pragma once

#include "mcc/ui/map/drawables/Point.h"
#include "mcc/ui/map/drawables/Label.h"

#include <chrono>
#include <iomanip>
#include <ctime>

namespace mcc {
namespace ui {
namespace map {

class LostSignalMarker : public Point {
public:
    LostSignalMarker(const core::LatLon& latLon, const std::chrono::system_clock::time_point& time,
                     const MapRectConstPtr& mapRect);
    LostSignalMarker(const QPointF& position, const core::LatLon& latLon,
                     const std::chrono::system_clock::time_point& time);

    void draw(QPainter* p, const QPixmap& lostSignalPixmap, const MapRectConstPtr& rect) const;
    const std::chrono::system_clock::time_point& time() const;

private:
    void update(const core::LatLon& latLon, const std::chrono::system_clock::time_point& time);
    LabelBase _label;
    std::chrono::system_clock::time_point _time;
};

inline LostSignalMarker::LostSignalMarker(const core::LatLon& latLon, const std::chrono::system_clock::time_point& time,
                                          const MapRectConstPtr& mapRect)
    : Point(WithPosition<>::fromLatLon(latLon, mapRect))
{
    update(latLon, time);
}

inline LostSignalMarker::LostSignalMarker(const QPointF& position, const core::LatLon& latLon,
                                          const std::chrono::system_clock::time_point& time)
    : Point(position)
{
    update(latLon, time);
}

inline void LostSignalMarker::update(const core::LatLon& latLon, const std::chrono::system_clock::time_point& time)
{
    _time = time;
    _label.setLabelAlignment(Qt::AlignBottom | Qt::AlignLeft);
    std::time_t a = std::chrono::system_clock::to_time_t(time);
    char output[13];
    std::strftime(output, sizeof(output), " (%H:%M:%S)", std::localtime(&a));
    QString coords
        = core::GlobalCoordinatePrinter::instance()->printer().print(latLon.latitude, latLon.longitude, "%1, %2");
    coords.append(output);
    _label.setLabel(coords);
}

inline const std::chrono::system_clock::time_point& LostSignalMarker::time() const
{
    return _time;
}

inline void LostSignalMarker::draw(QPainter* p, const QPixmap& lostSignalPixmap, const MapRectConstPtr& rect) const
{
    double height = lostSignalPixmap.height();
    double halfWidth = lostSignalPixmap.width() / 2;
    p->drawPixmap(position() + QPointF(-halfWidth, -height) - rect->mapOffsetRaw(), lostSignalPixmap);
    _label.drawAt(p, position() + QPointF(halfWidth, -height) - rect->mapOffsetRaw());
}
}
}
}
