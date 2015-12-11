/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#pragma once

#include "mcc/ui/map/DiskCache.h"
#include "mcc/ui/map/StaticMapType.h"
#include "mcc/ui/map/TilePosition.h"
#include "mcc/ui/map/MercatorProjection.h"
#include "mcc/ui/map/Ptr.h"

#include <string>
#include <functional>

namespace mcc {
namespace ui {
namespace map {

class MercatorProjection;
class FileCache;

class OnlineCache : public DiskCache {
public:
    OnlineCache();
    virtual ~OnlineCache();
    static OnlineCachePtr create(StaticMapType type);

    bool isBuiltIn() const override;
    int maxTileZoom() const override;
    std::string generateTileUrl(const TilePosition& pos) override;
    bool hasOnlineTiles() const override;
    const MercatorProjection& projection() const override;
    const QString& name() const override;
    const QString& description() const override;

    void setBasePath(const QString& path);
    StaticMapType type() const;

protected:
    typedef std::function<std::string(const TilePosition&)> Generator;

    virtual int calcMaxTileZoom() = 0;
    virtual const char* createServiceName() const = 0;
    virtual const char* createName() const = 0;
    virtual QString createDescription() const = 0;
    virtual const char* createFormat() const = 0;
    virtual MercatorProjection::ProjectionType createProjection() const = 0;
    virtual Generator createGenerator() const = 0;

private:
    MercatorProjection _proj;
    Generator _generator;
    int _maxTileZoom;
    QString _name;
    QString _description;
    StaticMapType _type;
};

inline StaticMapType OnlineCache::type() const
{
    return _type;
}

}
}
}
