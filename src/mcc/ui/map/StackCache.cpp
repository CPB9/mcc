/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "mcc/ui/map/StackCache.h"
#include "mcc/ui/map/FileCache.h"
#include "mcc/ui/map/OnlineCache.h"

namespace mcc {
namespace ui {
namespace map {

bool StackCache::tileExists(const TilePosition& pos) const
{
    for (const CacheElement& element : _caches) {
        if (element.isEnabled && element.cache->tileExists(pos)) {
            return true;
        }
    }
    return false;
}

QImage StackCache::readImage(const TilePosition& pos, const QRect& rect) const
{
    for (const CacheElement& element : _caches) {
        if (element.isEnabled) {
            QImage img = element.cache->readImage(pos, rect);
            if (!img.isNull()) {
                return img;
            }
        }
    }
    return QImage();
}

bool StackCache::saveTile(const TilePosition& pos, const bmcl::Buffer&) const
{
    Q_UNUSED(pos);
    return false;
}

static QString empty = "Стек карт";

const QString& StackCache::description() const
{
    return empty;
}

const QString& StackCache::name() const
{
    return empty;
}

const MercatorProjection& StackCache::projection() const
{
    return _proj;
}

bool StackCache::hasOnlineCache(StaticMapType type) const
{
    for (const CacheElement& element : _caches) {
        if (element.cache->isBuiltIn()) {
            const OnlineCache* onlineCache = static_cast<const OnlineCache*>(element.cache.get());
            if (onlineCache->type() == type) {
                return true;
            }
        }
    }
    return false;
}
}
}
}
