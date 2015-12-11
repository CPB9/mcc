/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#pragma once

#include "mcc/ui/map/layers/Layer.h"
#include "mcc/ui/core/Structs.h"
#include "mcc/ui/map/drawables/PolyLine.h"
#include "mcc/ui/map/drawables/Point.h"
#include "mcc/ui/map/drawables/Marker.h"
#include "mcc/ui/map/drawables/LostSignalMarker.h"

#include <QColor>

#include <chrono>

class QTime;

namespace mcc {
namespace ui {
namespace map {

class TailLayer : public Layer {
    Q_OBJECT
public:
    TailLayer(const MapRectConstPtr& mapRect, double tailParam = 20, core::TrailMode mode = core::TrailMode::Time);
    ~TailLayer() override;
    void draw(QPainter* p) const override;
    bool mouseMoveEvent(const QPoint& oldPos, const QPoint& newPos) override;
    bool mousePressEvent(const QPoint& pos) override;
    bool mouseReleaseEvent(const QPoint& pos) override;
    bool viewportResizeEvent(const QSize& oldSize, const QSize& newSize) override;
    bool viewportScrollEvent(const QPoint& oldPos, const QPoint& newPos) override;
    bool contextMenuEvent(const QPoint& pos) override;
    bool zoomEvent(const QPoint& pos, int fromZoom, int toZoom) override;
    void changeProjection(const MercatorProjection& from, const MercatorProjection& to) override;
    bool viewportResetEvent(int oldZoom, int newZoom, const QRect& oldViewpiort, const QRect& newViewport) override;

    void setSignalLost();
    void addPoint(const core::GeoPosition& position, const QDateTime& time);
    void setLineColor(const QColor& color);
    void setLineWidth(int width);
    void setTrailMode(core::TrailMode mode);
    void setMaxTailParam(double param);
    void setTrailModeAndParam(core::TrailMode mode, double param);

    void clear();
private:
    struct TailPointInfo {
        TailPointInfo(const core::GeoPosition& pos, const std::chrono::system_clock::time_point& time)
            : position(pos)
            , time(time)
            , lostSignal(false)
        {
        }

        void setSignalLost(bool isLost)
        {
            lostSignal = isLost;
        }

        bool signalIsLost() const
        {
            return lostSignal;
        }

        core::GeoPosition position;
        std::chrono::system_clock::time_point time;
        bool lostSignal;
    };

    void addPointNoCut(const core::GeoPosition& position, const QDateTime& time);
    void addPointNoCut(const core::GeoPosition& position, const std::chrono::system_clock::time_point& time);
    void cutToDistance();
    void cutToTime();
    void cut();
    void cutMarkers();

    Marker _lostMarker;
    std::vector<TailPointInfo> _storage;
    PolyLineBase<Point, DistanceWrapper<EmptyWrapper<Point>>> _tail;
    std::deque<LostSignalMarker> _markers;
    core::TrailMode _mode;
    double _maxParam;
};
}
}
}
