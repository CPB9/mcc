/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#pragma once

#include "mcc/ui/map/FileCache.h"
#include "mcc/ui/map/MercatorProjection.h"
#include "mcc/ui/map/TilePosition.h"
#include "mcc/ui/map/TilePosCache.h"
#include "mcc/ui/map/Ptr.h"
#include "mcc/misc/Result.h"

#include <QString>
#include <QImage>
#include <QFile>

#include <cstdint>

namespace mcc {
namespace ui {
namespace map {

class OmcfCache : public FileCache {
public:
    enum Result { Ok, NoFilesFound, WriteError };

    Result open(const QString& path);

    static misc::Result<OmcfCachePtr, Result> create(const QString& path);

    int size() const;

    const MercatorProjection& projection() const override;
    const QString& description() const override;
    const QString& name() const override;
    bool saveTile(const TilePosition& pos, const bmcl::Buffer& img) const override;
    QImage readImage(const TilePosition& pos, const QRect& rect) const override;
    bool tileExists(const TilePosition& pos) const override;

    const QString& path() const;

private:
    struct TileFileInfo {
        int64_t offset;
        int64_t size;
    };

    FastTilePosCache<TileFileInfo> _cache;
    MercatorProjection _proj;
    QFile _file;
    QString _name;
    QString _description;
    QString _path;
    const uchar* _mapped;
};

inline int OmcfCache::size() const
{
    return _cache.count();
}
}
}
}
