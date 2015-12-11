/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#pragma once

#include "mcc/ui/map/FileCache.h"
#include "mcc/ui/map/MercatorProjection.h"
#include "mcc/ui/map/StaticMapType.h"
#include "mcc/ui/map/Ptr.h"

#include <vector>

namespace mcc {
namespace ui {
namespace map {

class StackCache : public FileCache {
public:
    StackCache();
    static StackCachePtr create();

    const FileCacheConstPtr& at(std::size_t index) const;
    void append(const FileCacheConstPtr& cache, bool isEnabled = true);
    void insertAt(const FileCacheConstPtr& cache, std::size_t index, bool isEnabled = true);
    void removeAt(std::size_t index);
    void setEnabled(std::size_t index, bool flag);
    bool isEnabled(std::size_t index) const;
    std::size_t size() const;
    void swap(std::size_t left, std::size_t right);
    void setProjection(const MercatorProjection& proj);

    const MercatorProjection& projection() const override;
    const QString& description() const override;
    const QString& name() const override;
    bool saveTile(const TilePosition& pos, const bmcl::Buffer& img) const override;
    QImage readImage(const TilePosition& pos, const QRect& rect) const override;
    bool tileExists(const TilePosition& pos) const override;

    bool hasOnlineCache(StaticMapType type) const;

private:
    struct CacheElement {
        CacheElement(const FileCacheConstPtr& cache, bool isEnabled)
            : cache(cache)
            , isEnabled(isEnabled)
        {
        }
        FileCacheConstPtr cache;
        bool isEnabled;
    };
    std::vector<CacheElement> _caches;
    MercatorProjection _proj;
};

inline StackCache::StackCache()
{
}

inline StackCachePtr StackCache::create()
{
    return std::make_shared<StackCache>();
}

inline void StackCache::setProjection(const MercatorProjection& proj)
{
    _proj = proj;
}

inline const FileCacheConstPtr& StackCache::at(std::size_t index) const
{
    return _caches[index].cache;
}

inline void StackCache::append(const FileCacheConstPtr& cache, bool isEnabled)
{
    _caches.emplace_back(cache, isEnabled);
}

inline void StackCache::swap(std::size_t left, std::size_t right)
{
    std::swap(_caches[left], _caches[right]);
}

inline void StackCache::insertAt(const FileCacheConstPtr& cache, std::size_t index, bool isEnabled)
{
    _caches.emplace(_caches.begin() + index, cache, isEnabled);
}

inline void StackCache::removeAt(std::size_t index)
{
    _caches.erase(_caches.begin() + index);
}

inline bool StackCache::isEnabled(std::size_t index) const
{
    return _caches[index].isEnabled;
}

inline std::size_t StackCache::size() const
{
    return _caches.size();
}

inline void StackCache::setEnabled(std::size_t index, bool flag)
{
    _caches[index].isEnabled = flag;
}
}
}
}
