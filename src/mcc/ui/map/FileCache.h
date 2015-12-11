/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#pragma once

#include "mcc/ui/map/TilePosition.h"
#include "mcc/ui/map/WebMapProperties.h"
#include "mcc/ui/map/Ptr.h"
#include "mcc/misc/Option.h"

#include <QString>
#include <QImage>

namespace bmcl {
class Buffer;
}

namespace mcc {
namespace ui {
namespace map {

class MercatorProjection;

class FileCache {
public:
    enum TileType { Original, Scaled, Empty };
    FileCache(const char* format = "jpg");
    virtual ~FileCache();

    static QString createPath(const TilePosition& pos, const char* format);
    static misc::Option<TilePosition> createPosition(const QString& relativePath, const char* format);

    std::pair<QImage, FileCache::TileType> loadTile(const TilePosition& pos) const;
    QString createPath(const TilePosition& pos) const;
    misc::Option<TilePosition> createPosition(const QString& relativePath) const;
    const char* format() const;
    void setFormat(const char* format);

    virtual bool isBuiltIn() const;
    virtual int maxTileZoom() const;
    virtual std::string generateTileUrl(const TilePosition& pos);
    virtual bool hasOnlineTiles() const;
    virtual const MercatorProjection& projection() const = 0;
    virtual const QString& description() const = 0;
    virtual const QString& name() const = 0;
    virtual bool saveTile(const TilePosition& pos, const bmcl::Buffer& img) const = 0;
    virtual QImage readImage(const TilePosition& pos, const QRect& rect) const = 0;
    virtual bool tileExists(const TilePosition& pos) const = 0;

private:
    const char* _format;
};

inline FileCache::FileCache(const char* format)
    : _format(format)
{
}

inline QString FileCache::createPath(const TilePosition& pos) const
{
    return createPath(pos, _format);
}

inline misc::Option<TilePosition> FileCache::createPosition(const QString& relativePath) const
{
    return createPosition(relativePath, _format);
}

inline const char* FileCache::format() const
{
    return _format;
}

inline void FileCache::setFormat(const char* format)
{
    _format = format;
}
}
}
}
