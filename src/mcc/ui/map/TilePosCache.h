/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#pragma once

#include "mcc/ui/map/TilePosition.h"
#include "mcc/misc/Option.h"

#include <cstdint>
#include <map>
#include <unordered_map>

namespace mcc {
namespace ui {
namespace map {

template <typename T, typename F>
class TilePosCacheBase {
public:
    TilePosCacheBase();

    void addValue(const TilePosition& pos, const T& value);
    misc::Option<T> get(const TilePosition& pos) const;
    std::size_t count() const;
    std::size_t zoomLevelCount() const;
    const F& cache() const;

    template <typename C>
    void map(C callable) const;

private:
    std::size_t _count;
    F _cache;
};

template <typename T, typename F>
inline TilePosCacheBase<T, F>::TilePosCacheBase()
    : _count(0)
{
}

template <typename T, typename F>
inline void TilePosCacheBase<T, F>::addValue(const TilePosition& pos, const T& value)
{
    _cache[pos.zoomLevel][pos.globalOffsetX][pos.globalOffsetY] = value;
    _count++;
}

template <typename T, typename F>
misc::Option<T> TilePosCacheBase<T, F>::get(const TilePosition& pos) const
{
    auto zit = _cache.find(pos.zoomLevel);
    if (zit != _cache.end()) {
        auto xit = zit->second.find(pos.globalOffsetX);
        if (xit != zit->second.end()) {
            auto yit = xit->second.find(pos.globalOffsetY);
            if (yit != xit->second.end()) {
                return yit->second;
            }
        }
    }
    return misc::None;
}

template <typename T, typename F>
template <typename C>
void TilePosCacheBase<T, F>::map(C callable) const
{
    TilePosition pos;
    std::size_t i = 0;
    for (const auto& zPair : _cache) {
        pos.zoomLevel = zPair.first;
        for (const auto& xPair : zPair.second) {
            pos.globalOffsetX = xPair.first;
            for (const auto& yPair : xPair.second) {
                pos.globalOffsetY = yPair.first;
                callable(pos, yPair.second, i);
                i++;
            }
        }
    }
}

template <typename T, typename F>
inline std::size_t TilePosCacheBase<T, F>::count() const
{
    return _count;
}

template <typename T, typename F>
inline std::size_t TilePosCacheBase<T, F>::zoomLevelCount() const
{
    return _cache.length();
}

template <typename T, typename F>
inline const F& TilePosCacheBase<T, F>::cache() const
{
    return _cache;
}

template <typename T>
using FastTilePosCache
    = TilePosCacheBase<T, std::unordered_map<int, std::unordered_map<int, std::unordered_map<int, T>>>>;

template <typename T>
using OrderedTilePosCache = TilePosCacheBase<T, std::map<int, std::map<int, std::map<int, T>>>>;
}
}
}
