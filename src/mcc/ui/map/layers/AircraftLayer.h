/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#pragma once

#include "mcc/ui/map/layers/Layer.h"
#include "mcc/ui/map/drawables/Marker.h"
#include "mcc/ui/core/Structs.h"

#include <memory>

class QTimer;

namespace mcc {
namespace ui {
namespace core {
class FlyingDevice;
}
namespace map {

class TailLayer;

class AircraftLayer : public Layer {
    Q_OBJECT
public:
    AircraftLayer(mcc::ui::core::FlyingDevice* model, const MapRectConstPtr& rect);
    ~AircraftLayer() override;
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

    bool hasDeviceAt(const QPoint& pos);
    void setActive(bool isActive);

private slots:
    void updateAccuracy();

private:
    void drawLabel(QPainter* p) const;
    void repaintAircraftPixmap();
    mcc::ui::core::FlyingDevice* _model;
    std::unique_ptr<TailLayer> _tail;
    std::unique_ptr<TailLayer> _dogTail;
    std::unique_ptr<QTimer> _blinkTimer;
    Marker _aircraftMarker;
    Point _dogPos;
    core::GeoPosition _lastPosition;
    core::GeoOrientation _lastOrientation;
    QImage _aircraftImage;
    int _accuracyInPixels;
    bool _isActive;
    bool _positionValid;
    bool _dogValid;
    bool _isBlinkState;
};
}
}
}
