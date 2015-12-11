/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#pragma once

#include <QPointF>

#include <cmath>

namespace mcc {
namespace ui {
namespace map {

template <typename B = void>
class Zoomable {
public:
    void changeZoomLevel(int from, int to);

    static void zoomIn(QPointF* point);
    static void zoomOut(QPointF* point);
    static QPointF zoomed(const QPointF& point, int oldZoom, int newZoom);
    static double calcZoomRatio(int oldZoom, int newZoom);
};

template <typename B>
inline void Zoomable<B>::changeZoomLevel(int from, int to)
{
    return static_cast<B*>(this)->changeZoomLevel(from, to);
}

template <typename B>
inline void Zoomable<B>::zoomIn(QPointF* point)
{
    *point *= 2;
}

template <typename B>
inline void Zoomable<B>::zoomOut(QPointF* point)
{
    *point /= 2;
}

template <typename B>
inline double Zoomable<B>::calcZoomRatio(int fromZoomLevel, int toZoomLevel)
{
    int delta = toZoomLevel - fromZoomLevel;
    return std::exp2(delta);
}

template <typename B>
inline QPointF Zoomable<B>::zoomed(const QPointF& point, int oldZoom, int newZoom)
{
    return point * calcZoomRatio(oldZoom, newZoom);
}
}
}
}
