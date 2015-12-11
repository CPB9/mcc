/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#pragma once

#include "mcc/ui/map/MapRect.h"
#include "mcc/ui/map/layers/Layer.h"
#include "mcc/ui/map/TilePosition.h"
#include "mcc/ui/map/MemoryCache.h"
#include "mcc/ui/map/FileCache.h"
#include "mcc/ui/map/StaticMapType.h"
#include "mcc/misc/Channel.h"
#include "mcc/misc/ChannelConsumer.h"

#include <bmcl/Buffer.h>

#include <QObject>
#include <QPixmap>

#include <deque>
#include <mutex>
#include <utility>

namespace mcc {
namespace ui {
namespace map {

class TileLoader;
class CurlDownloader;
class MercatorProjection;

class MapLayer : public Layer {
    Q_OBJECT
public:
    MapLayer(const FileCachePtr& mapInfo, const MapRectConstPtr& rect, QObject* parent = 0);
    ~MapLayer() override;

    void draw(QPainter* p) const override;
    bool mousePressEvent(const QPoint& pos) override;
    bool mouseReleaseEvent(const QPoint& pos) override;
    bool mouseMoveEvent(const QPoint& oldPos, const QPoint& newPos) override;
    bool viewportResizeEvent(const QSize& oldSize, const QSize& newSize) override;
    bool viewportScrollEvent(const QPoint& oldPos, const QPoint& newPos) override;
    bool contextMenuEvent(const QPoint& pos) override;
    bool zoomEvent(const QPoint& pos, int fromZoom, int toZoom) override;
    bool viewportResetEvent(int oldZoom, int newZoom, const QRect& oldViewpiort, const QRect& newViewport) override;
    void changeProjection(const MercatorProjection& from, const MercatorProjection& to) override;

    void setMapInfo(const FileCachePtr& mapInfo);
    void enableTileDownloading(bool flag);

public slots:
    void reload();

private slots:
    void onResize(const QSize& size);
    void onScroll(int dx, int dy);
    void onRectChanged(int zoomLevel, const QSize& size, const QPoint& offset);

private:
    void startTasks();
    void stopTasks();
    void downloadNext(const std::shared_ptr<CurlDownloader>& downloader, const TilePosition& pos);
    void clear();
    double relativeOffset(int y) const;
    void adjustMapOffsets();
    void loadImages(const std::vector<TilePosition>& images);
    void updateImages(const std::vector<TilePosition>& images);

    bool _downloadEnabled;

    QPoint _paintOffset;

    FileCachePtr _mapInfo;
    MemoryCache _cache;
    mutable std::mutex _cacheMutex;

    std::vector<std::shared_ptr<CurlDownloader>> _downloaders;

    misc::Channel<std::vector<TilePosition>> _inputQueue;
    misc::Channel<TilePosition> _downloadQueue;
    misc::Channel<std::pair<TilePosition, QImage>> _resultQueue;
    misc::Channel<std::pair<TilePosition, bmcl::Buffer>> _saveQueue;
    misc::ChannelConsumer _tasks;
};
}
}
}
