/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "mcc/ui/map/FileCache.h"

#include <QRect>
#include <QDir>
#include <QFileInfo>
#include <QImageReader>
#include <QString>
#include <QDebug>

#include <cmath>

namespace mcc {
namespace ui {
namespace map {

using namespace mcc::misc;

FileCache::~FileCache()
{
}

QString FileCache::createPath(const TilePosition& pos, const char* format)
{
    QChar sep = '/';
    QString path;
    path.reserve(52); // достаточно - 52 один из размеров роста QString
    path.append('z');
    path.append(QString::number(pos.zoomLevel));
    path.append(sep);
    path.append(QString::number(pos.globalOffsetX / 1024));
    path.append(sep);
    path.append('x');
    path.append(QString::number(pos.globalOffsetX));
    path.append(sep);
    path.append(QString::number(pos.globalOffsetY / 1024));
    path.append(sep);
    path.append('y');
    path.append(QString::number(pos.globalOffsetY));
    path.append('.');
    path.append(format);
    return path;
}

static int parse(const QString& str, char c, int min, int max)
{
    if (str[0] != c) {
        return -1;
    }
    bool isOk;
    int zoom = str.midRef(1).toInt(&isOk);
    if (!isOk || zoom < min || zoom > max) {
        return -1;
    }
    return zoom;
}

static int parseXY(const QString& subDirStr, const QString& xyStr, char c, int min, int max)
{
    bool isOk;
    int xySubDir = subDirStr.toInt(&isOk);
    if (!isOk || xySubDir < 0) {
        return -1;
    }

    int xy = parse(xyStr, c, min, max);
    if (xy < 0 || ((xy / 1024) != xySubDir)) {
        return -1;
    }
    return xy;
}

Option<TilePosition> FileCache::createPosition(const QString& relativePath, const char* format)
{
    QChar sep = '/'; // все пути созадаемые в qt используют разделитель /
    QStringList subStrs = relativePath.split(sep, QString::SkipEmptyParts);
    if (subStrs.length() != 5) {
        return None;
    }

    int zoom = parse(subStrs[0], 'z', 1, 30);
    if (zoom < 0) {
        return None;
    }

    int maxTileIndex = std::exp2(zoom - 1) - 1;
    int x = parseXY(subStrs[1], subStrs[2], 'x', 0, maxTileIndex);
    if (x < 0) {
        return None;
    }

    QStringList ySubStrs = subStrs[4].split('.', QString::SkipEmptyParts);
    if (ySubStrs.length() == 2) {
        if (ySubStrs[1] != format) {
            return None;
        }
    } else {
        return None;
    }
    int y = parseXY(subStrs[3], ySubStrs[0], 'y', 0, maxTileIndex);
    if (y < 0) {
        return None;
    }

    return TilePosition(zoom, x, y);
}

std::pair<QImage, FileCache::TileType> FileCache::loadTile(const TilePosition& pos) const
{
    QRect rect = QRect(0, 0, 256, 256);
    TilePosition newPos = pos;
    int relativeZoom = 1;

    while (newPos.zoomLevel > 1 && !tileExists(newPos)) {
        if (relativeZoom <= 128) {
            relativeZoom *= 2;
        }
        int offsetX = pos.globalOffsetX % relativeZoom;
        int offsetY = pos.globalOffsetY % relativeZoom;
        int size = 256 / relativeZoom;
        rect = QRect(offsetX * size, offsetY * size, size, size);
        newPos.zoomLevel--;
        newPos.globalOffsetX /= 2;
        newPos.globalOffsetY /= 2;
    }

    QImage img = readImage(newPos, rect);
    if (img.isNull()) {
        return std::pair<QImage, TileType>(std::move(img), FileCache::Empty);
    }
    if (relativeZoom == 1) {
        return std::pair<QImage, TileType>(std::move(img), FileCache::Original);
    }
    QImage scaled = img.scaled(256, 256, Qt::IgnoreAspectRatio, Qt::FastTransformation);
    return std::pair<QImage, TileType>(std::move(scaled), FileCache::Scaled);
}

bool FileCache::isBuiltIn() const
{
    return false;
}

int FileCache::maxTileZoom() const
{
    return WebMapProperties::maxZoom();
}

std::string FileCache::generateTileUrl(const TilePosition& pos)
{
    (void)pos;
    return std::string();
}

bool FileCache::hasOnlineTiles() const
{
    return false;
}
}
}
}
