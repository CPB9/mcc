/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#pragma once

#include "mcc/ui/map/FileCache.h"
#include "mcc/ui/map/TilePosition.h"
#include "mcc/ui/map/MercatorProjection.h"
#include "mcc/misc/Result.h"

#include <QString>
#include <QImage>

namespace mcc {
namespace ui {
namespace map {

class DiskCache : public FileCache {
public:
    DiskCache(const QString& cachePath, const QString& subdir = QString(), const char* format = "jpg");

    bool saveTile(const TilePosition& pos, const bmcl::Buffer& img) const override;
    QImage readImage(const TilePosition& pos, const QRect& rect) const override;
    bool tileExists(const TilePosition& pos) const override;

protected:
    void setPath(const QString& basePath, const QString& subdir = QString());
    const QString& path() const;

private:
    QString _cachePath;
};

inline DiskCache::DiskCache(const QString& cachePath, const QString& subdir, const char* format)
    : FileCache(format)
{
    setPath(cachePath, subdir);
}
}
}
}
