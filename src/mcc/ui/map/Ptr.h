/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#pragma once

#include <memory>

namespace mcc {
namespace ui {
namespace map {

class FileCache;

class MapRect;
class OmcfCache;
class OnlineCache;
class StackCache;

class Layer;

class DeviceLayer;
class FlagLayer;
class GridLayer;
class InfoLayer;
class KmlModelLayer;
class LayerGroup;
class MapLayer;
class RulerLayer;
class HomeLayer;

typedef std::shared_ptr<const FileCache> FileCacheConstPtr;
typedef std::shared_ptr<const MapRect> MapRectConstPtr;

typedef std::shared_ptr<FileCache> FileCachePtr;
typedef std::shared_ptr<MapRect> MapRectPtr;
typedef std::shared_ptr<OmcfCache> OmcfCachePtr;
typedef std::shared_ptr<OnlineCache> OnlineCachePtr;
typedef std::shared_ptr<StackCache> StackCachePtr;

typedef std::shared_ptr<Layer> LayerPtr;

typedef std::shared_ptr<DeviceLayer> DeviceLayerPtr;
typedef std::shared_ptr<FlagLayer> FlagLayerPtr;
typedef std::shared_ptr<GridLayer> GridLayerPtr;
typedef std::shared_ptr<InfoLayer> InfoLayerPtr;
typedef std::shared_ptr<KmlModelLayer> KmlModelLayerPtr;
typedef std::shared_ptr<LayerGroup> LayerGroupPtr;
typedef std::shared_ptr<MapLayer> MapLayerPtr;
typedef std::shared_ptr<RulerLayer> RulerLayerPtr;
typedef std::shared_ptr<HomeLayer> HomeLayerPtr;
}
}
}
