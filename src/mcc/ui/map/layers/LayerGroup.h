/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#pragma once

#include "mcc/ui/map/layers/Layer.h"
#include "mcc/ui/core/Structs.h"
#include "mcc/ui/map/MapMode.h"
#include "mcc/ui/map/Ptr.h"
#include "mcc/misc/Option.h"

#include <vector>

namespace mcc {
namespace ui {
namespace core {
class DeviceManager;
}
namespace map {

class KmlModel;

class LayerGroup : public Layer {
    Q_OBJECT
public:
    LayerGroup(const OnlineCachePtr& onlineCache, core::DeviceManager* manager, KmlModel* kmlModel, const MapRectConstPtr& rect);
    ~LayerGroup() override;
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

    const MapLayerPtr& mapLayer() const;
    const DeviceLayerPtr& deviceLayer() const;
    const HomeLayerPtr& homeLayer() const;
    misc::Option<core::GeoBox> setMode(MapMode mode);
    MapMode mode() const;

private:
    void selectLayer(const LayerPtr& layer);
    template <typename F, typename... A>
    void visitLayers(F&& func, A&&... args);
    LayerPtr _activeLayer;
    MapLayerPtr _mapLayer;
    InfoLayerPtr _infoLayer;
    FlagLayerPtr _flagLayer;
    DeviceLayerPtr _deviceLayer;
    RulerLayerPtr _rulerLayer;
    KmlModelLayerPtr _kmlLayer;
    GridLayerPtr _gridLayer;
    HomeLayerPtr _homeLayer;
    std::vector<LayerPtr> _layers;
    std::vector<LayerPtr> _layersTemplate;
    MapMode _mode;
};

inline MapMode LayerGroup::mode() const
{
    return _mode;
}

inline const MapLayerPtr& LayerGroup::mapLayer() const
{
    return _mapLayer;
}

inline const DeviceLayerPtr& LayerGroup::deviceLayer() const
{
    return _deviceLayer;
}

inline const HomeLayerPtr& LayerGroup::homeLayer() const
{
    return _homeLayer;
}
}
}
}
