/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "mcc/ui/map/MemoryCache.h"

#include <cmath>

#include <QPainter>
#include <QDebug>

namespace mcc {
namespace ui {
namespace map {

MemoryCache::MemoryCache()
    : _zoomLevel(1)
    , _width(0)
    , _height(0)
    , _maxWidth(1)
    , _maxHeight(1)
    , _globalOffsetX(0)
    , _globalOffsetY(0)
{
    updateMaxSize();
}

void MemoryCache::draw(QPainter* p) const
{
    int pixmapOffsetX = 0;
    int pixmapOffsetY = 0;
    for (int tileY = 0; tileY < _height; tileY++) {
        pixmapOffsetX = 0;
        for (int tileX = 0; tileX < _width; tileX++) {
            const QPixmap& current = pixmapAt(tileX, tileY);
            p->drawPixmap(pixmapOffsetX, pixmapOffsetY, current);
            pixmapOffsetX += 256;
        }
        pixmapOffsetY += 256;
    }
}

void MemoryCache::drawNonTiled(QPainter* p) const
{
    int pixmapOffsetX = 0;
    int pixmapOffsetY = 0;
    for (const std::deque<QPixmap>& row : _cache) {
        pixmapOffsetX = 0;
        for (const QPixmap& pixmap : row) {
            p->drawPixmap(pixmapOffsetX, pixmapOffsetY, pixmap);
            pixmapOffsetX += 256;
        }
        pixmapOffsetY += 256;
    }
}

void MemoryCache::updatePixmap(const mcc::ui::map::TilePosition& pos, const QPixmap& image)
{
    if (pos.zoomLevel != _zoomLevel) {
        return;
    }

    int y = absOffset(pos.globalOffsetY - _globalOffsetY);
    int x = absOffset(pos.globalOffsetX - _globalOffsetX);

    for (int j = y; j < _height; j += _maxSize) {
        std::deque<QPixmap>& pixmapsRow = _cache[j];
        for (int i = x; i < _width; i += _maxSize) {
            pixmapsRow[i] = image;
        }
    }
}

const QPixmap& MemoryCache::pixmapAt(int x, int y) const
{
    if (y > _height - 1) {
        return _emptyPixmap;
    }
    const std::deque<QPixmap>& pixmapsRow = _cache[y];
    if (x > _width - 1) {
        return _emptyPixmap;
    }
    return pixmapsRow[x];
}

std::vector<TilePosition> MemoryCache::setSize(int tileCountX, int tileCountY)
{
    tileCountY = std::min(tileCountY, _maxSize);
    tileCountX = std::min(tileCountX, _maxSize);
    int dx = tileCountX - _width;
    int dy = tileCountY - _height;
    std::vector<TilePosition> queue;

    if (dy > 0) {
        queue.reserve(dy * _width);
        for (int i = 0; i < dy; i++) {
            _cache.emplace_back();
            populateEmptyRow(_cache.back());
            loadRow(_cache.size() - 1, queue);
        }
    } else if (dy < 0) {
        dy = -dy;
        for (int i = 0; i < dy; i++) {
            _cache.pop_back();
        }
    }

    if (dx > 0) {
        queue.reserve(queue.size() + dx * _height);
        for (int i = 0; i < dx; i++) {
            for (auto& row : _cache) {
                row.push_back(_emptyPixmap);
            }
            loadColumn(_cache[0].size() - 1, queue);
        }
    } else if (dx < 0) {
        dx = -dx;
        for (auto& row : _cache) {
            for (int i = 0; i < dx; i++) {
                row.pop_back();
            }
        }
    }

    _height = tileCountY;
    _width = tileCountX;

    return queue;
}

std::vector<TilePosition> MemoryCache::resize(int tileCountX, int tileCountY)
{
    _maxHeight = tileCountY;
    _maxWidth = tileCountX;
    return setSize(tileCountX, tileCountY);
}

std::vector<TilePosition> MemoryCache::scrollUp()
{
    std::vector<TilePosition> queue;
    _globalOffsetY = absOffset(_globalOffsetY - 1);
    if (_maxHeight == _maxSize) {
        std::deque<QPixmap> last = std::move(_cache.back());
        _cache.pop_back();
        _cache.push_front(std::move(last));
    } else {
        _cache.pop_back();
        _cache.emplace_front();
        populateEmptyRow(_cache.front());
        queue.reserve(_width);
        loadRow(0, queue);
    }
    return queue;
}

std::vector<TilePosition> MemoryCache::scrollDown()
{
    std::vector<TilePosition> queue;
    _globalOffsetY = absOffset(_globalOffsetY + 1);
    if (_maxHeight == _maxSize) {
        std::deque<QPixmap> first = std::move(_cache.front());
        _cache.pop_front();
        _cache.push_back(std::move(first));
    } else {
        _cache.pop_front();
        _cache.emplace_back();
        populateEmptyRow(_cache.back());
        queue.reserve(_width);
        loadRow(_height - 1, queue);
    }
    return queue;
}

std::vector<TilePosition> MemoryCache::scrollLeft()
{
    std::vector<TilePosition> queue;
    _globalOffsetX = absOffset(_globalOffsetX - 1);
    if (_width >= _maxSize) {
        for (std::deque<QPixmap>& row : _cache) {
            QPixmap last = std::move(row.back());
            row.pop_back();
            row.push_front(std::move(last));
        }
    } else {
        queue.reserve(_height);
        for (std::deque<QPixmap>& row : _cache) {
            row.pop_back();
            row.push_front(_emptyPixmap);
        }
        loadColumn(0, queue);
    }
    return queue;
}

std::vector<TilePosition> MemoryCache::scrollRight()
{
    std::vector<TilePosition> queue;
    _globalOffsetX = absOffset(_globalOffsetX + 1);
    if (_width >= _maxSize) {
        for (std::deque<QPixmap>& row : _cache) {
            QPixmap first = std::move(row.front());
            row.pop_front();
            row.push_back(std::move(first));
        }
    } else {
        queue.reserve(_height);
        for (std::deque<QPixmap>& row : _cache) {
            row.pop_front();
            row.push_back(_emptyPixmap);
        }
        loadColumn(_width - 1, queue);
    }
    return queue;
}

std::vector<TilePosition> MemoryCache::setPosition(int zoomLevel, int globalOffsetX, int globalOffsetY)
{
    if (zoomLevel < 1) {
        return std::vector<TilePosition>();
    }
    _zoomLevel = zoomLevel;
    updateMaxSize();
    setSize(std::min(_maxWidth, _maxSize), std::min(_maxHeight, _maxSize));
    _globalOffsetX = absOffset(globalOffsetX);
    _globalOffsetY = absOffset(globalOffsetY);
    return reloadCache();
}

std::vector<TilePosition> MemoryCache::setOffset(int globalOffsetX, int globalOffsetY)
{
    _globalOffsetX = absOffset(globalOffsetX);
    _globalOffsetY = absOffset(globalOffsetY);
    return reloadCache();
}

int MemoryCache::absOffset(int globalOffset) const
{
    if (_zoomLevel == 1) {
        return 0;
    }
    if (globalOffset < 0) {
        return _maxSize + globalOffset % _maxSize;
    }
    return globalOffset % _maxSize;
}

std::vector<TilePosition> MemoryCache::reloadCache()
{
    std::vector<TilePosition> queue;
    queue.reserve(_height * _width);
    for (int i = 0; i < _height; i++) {
        loadRow(i, queue);
    }
    return queue;
}

void MemoryCache::loadRow(int row, std::vector<TilePosition>& queue)
{
    int y = absOffset(_globalOffsetY + row);
    for (int i = 0; i < (int)_cache[0].size(); i++) {
        int x = absOffset(_globalOffsetX + i);
        queue.emplace_back(_zoomLevel, x, y);
    }
}

void MemoryCache::loadColumn(int column, std::vector<TilePosition>& queue)
{
    int x = absOffset(_globalOffsetX + column);
    for (int i = 0; i < (int)_cache.size(); i++) {
        int y = absOffset(_globalOffsetY + i);
        queue.emplace_back(_zoomLevel, x, y);
    }
}

void MemoryCache::populateEmptyRow(std::deque<QPixmap>& row)
{
    std::size_t size = _width;
    for (std::size_t i = 0; i < size; i++) {
        row.push_back(_emptyPixmap);
    }
}
}
}
}
