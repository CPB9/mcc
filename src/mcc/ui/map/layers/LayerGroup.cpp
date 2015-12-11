/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "mcc/ui/map/layers/LayerGroup.h"

#include "mcc/ui/map/layers/DeviceLayer.h"
#include "mcc/ui/map/layers/FlagLayer.h"
#include "mcc/ui/map/layers/GridLayer.h"
#include "mcc/ui/map/layers/InfoLayer.h"
#include "mcc/ui/map/layers/KmlModelLayer.h"
#include "mcc/ui/map/layers/MapLayer.h"
#include "mcc/ui/map/layers/RulerLayer.h"
#include "mcc/ui/map/layers/HomeLayer.h"
#include "mcc/ui/map/OnlineCache.h"

#include <QPainter>

namespace mcc {
namespace ui {
namespace map {

static const int subWidgetMargin = 10;
LayerGroup::LayerGroup(const OnlineCachePtr& onlineCache, core::DeviceManager* manager, KmlModel* kmlModel,
                       const MapRectConstPtr& rect)
    : Layer(rect)
    , _mode(MapMode::Default)
{
    _mapLayer = std::make_shared<MapLayer>(onlineCache, rect, mapRect()->parent());
    _infoLayer = std::make_shared<InfoLayer>(rect);
    _infoLayer->setMargins(subWidgetMargin, subWidgetMargin);
    _flagLayer = std::make_shared<FlagLayer>(rect);
    _gridLayer = std::make_shared<GridLayer>(rect);
    _deviceLayer = std::make_shared<DeviceLayer>(manager, rect);
    _rulerLayer = std::make_shared<RulerLayer>(rect);
    _kmlLayer = std::make_shared<KmlModelLayer>(kmlModel, rect);
    _homeLayer = std::make_shared<HomeLayer>(rect);
    _layers = {_mapLayer, _homeLayer, _kmlLayer, _infoLayer, _rulerLayer, _flagLayer, _deviceLayer};
    _layersTemplate = _layers;
    for (const LayerPtr& layer : _layers) {
        connect(layer.get(), &Layer::sceneUpdated, this, [this]() { emit sceneUpdated(); });
    }
    setMode(MapMode::Default);
}

LayerGroup::~LayerGroup()
{
}

void LayerGroup::selectLayer(const LayerPtr& layer)
{
    _activeLayer = layer;
    if (_mode != MapMode::Default) {
        _layers = _layersTemplate;
        _layers.erase(std::remove(_layers.begin(), _layers.end(), layer), _layers.end());
        _layers.push_back(layer);
    }
    _activeLayer = layer;
    _deviceLayer->setEditableLayerVisible(_mode == MapMode::Waypont);
}

void LayerGroup::draw(QPainter* p) const
{
    int maxMapSize = mapRect()->maxMapSize();
    int width = mapRect()->size().width();
    int x = mapRect()->mapOffsetRaw().x();
    QTransform t = p->transform();
    if ((x + width) > maxMapSize) {
        for (const LayerPtr& layer : _layers) {
            for (int i = 0; i < width; i += maxMapSize) {
                layer->draw(p);
                p->translate(maxMapSize, 0);
            }
            layer->draw(p);
            p->setTransform(t);
        }
    } else {
        for (const LayerPtr& layer : _layers) {
            for (int i = 0; i < width; i += maxMapSize) {
                layer->draw(p);
                p->translate(maxMapSize, 0);
            }
            p->setTransform(t);
        }
    }
}

template <typename F, typename... A>
inline void LayerGroup::visitLayers(F&& func, A&&... args)
{
    for (const LayerPtr& layer : _layers) {
        (layer.get()->*func)(std::forward<A>(args)...);
    }
}

void LayerGroup::changeProjection(const MercatorProjection& from, const MercatorProjection& to)
{
    visitLayers(&Layer::changeProjection, from, to);
}

bool LayerGroup::contextMenuEvent(const QPoint& pos)
{
    return _activeLayer->contextMenuEvent(pos);
}

bool LayerGroup::mouseMoveEvent(const QPoint& oldPos, const QPoint& newPos)
{
    return _activeLayer->mouseMoveEvent(oldPos, newPos);
}

bool LayerGroup::mousePressEvent(const QPoint& pos)
{
    return _activeLayer->mousePressEvent(pos);
}

bool LayerGroup::mouseReleaseEvent(const QPoint& pos)
{
    return _activeLayer->mouseReleaseEvent(pos);
}

bool LayerGroup::viewportResetEvent(int oldZoom, int newZoom, const QRect& oldViewpiort, const QRect& newViewport)
{
    visitLayers(&Layer::viewportResetEvent, oldZoom, newZoom, oldViewpiort, newViewport);
    return true;
}

bool LayerGroup::viewportResizeEvent(const QSize& oldSize, const QSize& newSize)
{
    visitLayers(&Layer::viewportResizeEvent, oldSize, newSize);
    return true;
}

bool LayerGroup::viewportScrollEvent(const QPoint& oldPos, const QPoint& newPos)
{
    visitLayers(&Layer::viewportScrollEvent, oldPos, newPos);
    return true;
}

bool LayerGroup::zoomEvent(const QPoint& pos, int fromZoom, int toZoom)
{
    visitLayers(&Layer::zoomEvent, pos, fromZoom, toZoom);
    return true;
}

misc::Option<core::GeoBox> LayerGroup::setMode(MapMode mode)
{
    _mode = mode;
    switch (mode) {
    case MapMode::Default:
        selectLayer(_deviceLayer);
        break;
    case MapMode::Ruler:
        selectLayer(_rulerLayer);
        return _rulerLayer->bbox();
    case MapMode::Waypont:
        selectLayer(_deviceLayer);
        return _deviceLayer->currentBbox();
    case MapMode::Kml:
        selectLayer(_kmlLayer);
        break;
    case MapMode::Flag:
        selectLayer(_flagLayer);
        break;
    };
    return misc::None;
}
}
}
}
