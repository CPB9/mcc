/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#pragma once

#include <mcc/ui/map/layers/Layer.h>
#include <mcc/ui/map/layers/KmlElementLayer.h>

#include "kml/dom.h"

#include <map>
#include <memory>

class QPainter;
class QMenu;

namespace mcc {
namespace ui {
namespace map {

class KmlModel;

class KmlModelLayer : public Layer {
    Q_OBJECT
public:
    KmlModelLayer(KmlModel* model, const MapRectConstPtr& rect);
    ~KmlModelLayer() override;
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

private:
    template <typename F, typename... A>
    void visitLayers(F func, A&&... args);
    template <typename F, typename... A>
    bool visitCurrentLayer(F func, A&&... args);
    template <typename F, typename... A>
    void visitLayersUntilAccepts(F func, A&&... args);
    void initFrom(kmldom::Document* parent, const kmldom::ElementPtr& element);
    void addElement(kmldom::Document* parent, const kmldom::ElementPtr& element, bool isActive = false);
    std::shared_ptr<KmlElementLayer> createPlacemark(kmldom::Document* parent, const kmldom::PlacemarkPtr& pm,
                                                     const MapRectConstPtr& rect);
    KmlModel* _model;
    std::map<kmldom::ElementPtr, std::shared_ptr<KmlElementLayer>> _layers;
    kmldom::ElementPtr _currentSelected;
    std::shared_ptr<KmlElementLayer> _currentLayer;
};
}
}
}
