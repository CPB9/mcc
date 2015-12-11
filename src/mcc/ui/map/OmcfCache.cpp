/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "mcc/ui/map/OmcfCache.h"
#include "mcc/misc/Crc.h"

#include "bmcl/MemReader.h"

#include <QRect>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QBuffer>
#include <QImageReader>
#include <QString>
#include <QDebug>

#include <cstddef>
#include <unordered_map>

#include <cstdint>

namespace mcc {
namespace ui {
namespace map {

static const uint32_t headerMagic = 0x5a5a5a5a;

using namespace mcc::misc;

Result<OmcfCachePtr, OmcfCache::Result> OmcfCache::create(const QString& path)
{
    OmcfCachePtr cache = std::make_shared<OmcfCache>();
    Result rv = cache->open(path);
    if (rv != Ok) {
        return rv;
    }
    return cache;
}

const QString& OmcfCache::description() const
{
    return _description;
}

const QString& OmcfCache::name() const
{
    return _name;
}

OmcfCache::Result OmcfCache::open(const QString& path)
{
    _path = path;
    _file.setFileName(path);
    if (!_file.open(QIODevice::ReadOnly)) {
        return NoFilesFound;
    }
    _mapped = _file.map(0, _file.size());
    if (!_mapped) {
        qCritical() << "memory map failed";
        return NoFilesFound;
    }
    bmcl::MemReader header(_mapped, _file.size());
    uint32_t magic = header.readUint32Le();
    assert(magic == headerMagic);
    uint32_t headerSize = header.readUint32Le();
    uint32_t crc = crc32(_mapped, headerSize - 4);
    assert(crc == *(uint32_t*)(_mapped + headerSize - 4));
    double a;
    header.read(&a, sizeof(double));
    double b;
    header.read(&b, sizeof(double));
    _proj = MercatorProjection(a, b);
    uint32_t nameSize = header.readUint32Le();
    _name = QString((QChar*)header.current(), nameSize);
    header.skip(nameSize * sizeof(QChar));
    uint32_t descriptionSize = header.readUint32Le();
    _description = QString((QChar*)header.current(), descriptionSize);
    header.skip(descriptionSize * sizeof(QChar));
    uint32_t tilesNum = header.readUint32Le();
    for (uint32_t i = 0; i < tilesNum; i++) {
        TilePosition pos;
        pos.zoomLevel = header.readUint16Le();
        pos.globalOffsetX = header.readUint32Le();
        pos.globalOffsetY = header.readUint32Le();
        TileFileInfo info;
        info.offset = header.readUint64Le();
        info.size = header.readUint64Le();
        _cache.addValue(pos, info);
    }
    return Ok;
}

QImage OmcfCache::readImage(const TilePosition& pos, const QRect& rect) const
{
    auto info = _cache.get(pos);
    if (info.isNone()) {
        return QImage();
    }
    QBuffer buf;
    buf.setData((char*)(_mapped + info.unwrap().offset), info.unwrap().size);
    QImageReader reader(&buf);
    reader.setClipRect(rect);
    return reader.read();
}


bool OmcfCache::saveTile(const TilePosition& pos, const bmcl::Buffer&) const
{
    Q_UNUSED(pos);
    return false;
}

bool OmcfCache::tileExists(const TilePosition& pos) const
{
    return _cache.get(pos).isSome();
}

const MercatorProjection& OmcfCache::projection() const
{
    return _proj;
}

const QString& OmcfCache::path() const
{
    return _path;
}
}
}
}
