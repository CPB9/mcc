/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "mcc/ui/map/DiskCache.h"
#include <bmcl/Buffer.h>

#include <QRect>
#include <QDir>
#include <QFileInfo>
#include <QImageReader>
#include <QDebug>

namespace mcc {
namespace ui {
namespace map {

void DiskCache::setPath(const QString& basePath, const QString& subdir)
{
    _cachePath = basePath;
    _cachePath.append(QDir::separator());
    _cachePath.append(subdir);
    _cachePath.append(QDir::separator());
}

bool DiskCache::tileExists(const TilePosition& pos) const
{
    return QFileInfo::exists(_cachePath + createPath(pos));
}

QImage DiskCache::readImage(const TilePosition& pos, const QRect& rect) const
{
    QImageReader reader(_cachePath + createPath(pos));
    reader.setClipRect(rect);
    return reader.read();
}

bool DiskCache::saveTile(const TilePosition& pos, const bmcl::Buffer& img) const
{
    QString path = _cachePath + createPath(pos);
    QDir().mkpath(QFileInfo(path).absolutePath());

    QFile file(path);
    bool isOk = file.open(QIODevice::WriteOnly);
    if (!isOk) {
        return false;
    }

    int64_t size = file.write((const char*)img.start(), img.size());
    return size == img.size();
}
}
}
}
