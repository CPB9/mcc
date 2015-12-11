/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "mcc/ui/map/layers/MapLayer.h"
#include "mcc/ui/map/FileCache.h"
#include "mcc/ui/map/CurlDownloader.h"
#include "mcc/ui/map/MercatorProjection.h"
#include "bmcl/Utils.h"
#include "bmcl/Buffer.h"

#include <QImage>
#include <QPixmap>
#include <QPainter>
#include <QDebug>
#include <QPoint>
#include <QStandardPaths>

#include <cmath>

namespace mcc {
namespace ui {
namespace map {

using namespace mcc::misc;
using namespace std::placeholders;

const std::size_t nDefaultDownloadThreads = 6;
const int tileSize = 256;

MapLayer::MapLayer(const FileCachePtr& mapInfo, const MapRectConstPtr& rect, QObject* parent)
    : Layer(rect)
    , _downloadEnabled(false)
    , _paintOffset(0, 0)
    , _mapInfo(mapInfo)
{
    (void)parent;
    QImage image(tileSize, tileSize, QImage::Format_RGB32);
    QPainter p(&image);
    p.fillRect(0, 0, tileSize, tileSize, Qt::gray);
    QPixmap emptyPixmap = QPixmap::fromImage(image);
    _cache.setDefaultPixmap(emptyPixmap);

    startTasks();
    adjustMapOffsets();
}

MapLayer::~MapLayer()
{
    stopTasks();
}

bool MapLayer::mouseMoveEvent(const QPoint& oldPos, const QPoint& newPos)
{
    Q_UNUSED(oldPos);
    Q_UNUSED(newPos);
    return true;
}

bool MapLayer::mousePressEvent(const QPoint& pos)
{
    (void)pos;
    return false;
}

bool MapLayer::mouseReleaseEvent(const QPoint& pos)
{
    (void)pos;
    return false;
}

bool MapLayer::contextMenuEvent(const QPoint& pos)
{
    (void)pos;
    return false;
}

bool MapLayer::viewportResetEvent(int oldZoom, int newZoom, const QRect& oldViewpiort, const QRect& newViewport)
{
    (void)oldZoom;
    (void)oldViewpiort;
    onRectChanged(newZoom, newViewport.size(), mapRect()->mapOffset());
    return true;
}

bool MapLayer::viewportScrollEvent(const QPoint& oldPos, const QPoint& newPos)
{
    QPoint delta = newPos - oldPos;
    onScroll(delta.x(), delta.y());
    return true;
}

bool MapLayer::zoomEvent(const QPoint& pos, int fromZoom, int toZoom)
{
    (void)pos;
    (void)fromZoom;
    (void)toZoom;
    onRectChanged(mapRect()->zoomLevel(), mapRect()->size(), mapRect()->mapOffset());
    return true;
}

bool MapLayer::viewportResizeEvent(const QSize& oldSize, const QSize& newSize)
{
    (void)oldSize;
    onResize(newSize);
    return true;
}

void MapLayer::changeProjection(const MercatorProjection& from, const MercatorProjection& to)
{
    // FIXME
    (void)from;
    (void)to;
    onRectChanged(mapRect()->zoomLevel(), mapRect()->size(), mapRect()->mapOffset());
}

void MapLayer::startTasks()
{
    _inputQueue.reopen();
    _downloadQueue.reopen();
    _saveQueue.reopen();
    _resultQueue.reopen();
    _tasks.addWorker(&_saveQueue, [this](const std::pair<TilePosition, bmcl::Buffer>& pair) {
        _mapInfo->saveTile(pair.first, pair.second);
    });
    _tasks.addWorker(&_resultQueue, [this](const std::pair<TilePosition, QImage>& pair) {
        if (pair.second.isNull()) {
            std::lock_guard<std::mutex> lock(_cacheMutex);
            _cache.resetPixmap(pair.first);
        } else {
            std::lock_guard<std::mutex> lock(_cacheMutex);
            _cache.updatePixmap(pair.first, QPixmap::fromImage(pair.second));
        }
        emit sceneUpdated();
    });
    _tasks.addWorker(&_inputQueue, std::bind(&MapLayer::updateImages, this, _1));
    for (std::size_t i = 0; i < nDefaultDownloadThreads; i++) {
        std::shared_ptr<CurlDownloader> downloader = std::make_shared<CurlDownloader>();
        _downloaders.push_back(downloader);
        _tasks.addWorker(&_downloadQueue, std::bind(&MapLayer::downloadNext, this, downloader, _1));
    }
}

void MapLayer::stopTasks()
{
    _inputQueue.close();
    _downloadQueue.close();
    _saveQueue.close();
    _resultQueue.close();
    _tasks.clear();
    _downloaders.clear();
}

void MapLayer::enableTileDownloading(bool flag)
{
    _downloadEnabled = flag;
    if (flag) {
        auto queue = _cache.reloadCache();
        _inputQueue.send(queue);
    }
}

void MapLayer::setMapInfo(const FileCachePtr& mapInfo)
{
    stopTasks();
    _mapInfo = mapInfo;
    startTasks();
    loadImages(_cache.reloadCache());
}

void MapLayer::reload()
{
    // TODO: clear
    updateImages(_cache.reloadCache());
}

void MapLayer::clear()
{
    _downloadQueue.clear();
}

void MapLayer::loadImages(const std::vector<TilePosition>& positions)
{
    std::vector<TilePosition> tilesToUpdate;
    tilesToUpdate.reserve(positions.size());
    _cacheMutex.lock();
    for (const TilePosition& pos : positions) {
        auto pair = _mapInfo->loadTile(pos);
        if (_downloadEnabled && pair.second != FileCache::Original && pos.zoomLevel <= _mapInfo->maxTileZoom()) {
            tilesToUpdate.push_back(pos);
        }

        if (pair.second == FileCache::Empty) {
            _cache.resetPixmap(pos);
        } else {
            _cache.updatePixmap(pos, QPixmap::fromImage(pair.first));
        }
    }
    _cacheMutex.unlock();
    emit sceneUpdated();
    if (_downloadEnabled) {
        for (const TilePosition& pos : tilesToUpdate) {
            _downloadQueue.send(pos);
        }
    }
}

void MapLayer::updateImages(const std::vector<TilePosition>& positions)
{
    for (const TilePosition& pos : positions) {
        auto pair = _mapInfo->loadTile(pos);
        _resultQueue.sendEmplace(pos, std::move(pair.first));

        if (_downloadEnabled && pair.second != FileCache::Original && pos.zoomLevel <= _mapInfo->maxTileZoom()) {
            _downloadQueue.send(pos);
        }
    }
}

void MapLayer::downloadNext(const std::shared_ptr<CurlDownloader>& downloader, const TilePosition& pos)
{
    bmcl::Buffer data = downloader->download(_mapInfo->generateTileUrl(pos));
    QImage img = QImage::fromData(data.start(), data.size());
    if (!img.isNull()) {
        _resultQueue.sendEmplace(pos, img);
        _saveQueue.sendEmplace(pos, std::move(data));
    }
}

void MapLayer::draw(QPainter* p) const
{
    // p->fillRect(0, 0, _width, _height, Qt::red); //debug
    if (mapRect()->isCenteredVertically()) {
        p->fillRect(0, 0, mapRect()->size().width(), -_paintOffset.y(), Qt::gray);
        p->fillRect(0, -_paintOffset.y() + mapRect()->maxMapSize(), mapRect()->size().width(),
                    mapRect()->size().height(), Qt::gray);
    }
    auto t = p->transform();
    p->translate(-_paintOffset);
    _cacheMutex.lock();
    _cache.drawNonTiled(p);
    _cacheMutex.unlock();
    p->setTransform(t);
}

void MapLayer::onScroll(int dx, int dy)
{
    adjustMapOffsets();
    QPoint newGlobalOffset = QPoint(mapRect()->offset().x() / tileSize,
                                    mapRect()->offset().y() / tileSize); // qpoint / int не работает
    QPoint globalOffsetDelta = newGlobalOffset - _cache.globalOffset();
    if (globalOffsetDelta.x() != 0) {
        if (dx > 0) {
            _inputQueue.send(_cache.scrollLeft());
        } else if (dx < 0) {
            _inputQueue.send(_cache.scrollRight());
        }
    }
    if (globalOffsetDelta.y() != 0) {
        if (dy > 0) {
            _inputQueue.send(_cache.scrollUp());
        } else if (dy < 0) {
            _inputQueue.send(_cache.scrollDown());
        }
    }
}

void MapLayer::adjustMapOffsets()
{
    _paintOffset.rx() = mapRect()->offset().x() % tileSize;
    if (mapRect()->isCenteredVertically()) {
        _paintOffset.ry() = (mapRect()->maxMapSize() - mapRect()->size().height()) / 2;
    } else {
        _paintOffset.ry() = mapRect()->offset().y() % tileSize;
    }
}

void MapLayer::onRectChanged(int zoomLevel, const QSize& size, const QPoint& offset)
{
    Q_UNUSED(size);
    adjustMapOffsets();
    int tx = offset.x() / tileSize;
    int ty = offset.y() / tileSize;
    clear();
    loadImages(_cache.setPosition(zoomLevel, tx, ty));
}

void MapLayer::onResize(const QSize& size)
{
    Q_UNUSED(size);
    int tileCountX = mapRect()->size().width() / tileSize + 2;
    int tileCountY = mapRect()->size().height() / tileSize + 2;
    adjustMapOffsets();
    loadImages(_cache.resize(tileCountX, tileCountY));
}
}
}
}
