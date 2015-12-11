/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include <mcc/ui/map/OnlineCache.h>
#include <mcc/ui/map/DiskCache.h>
#include <mcc/ui/map/MercatorProjection.h>
#include <mcc/ui/map/WebMapProperties.h>
#include <mcc/ui/core/Settings.h>

#include <cassert>
#include <random>

namespace mcc {
namespace ui {
namespace map {

struct EmptyGen {
public:
    std::string operator()(const TilePosition& pos)
    {
        Q_UNUSED(pos);
        return std::string();
    }
};


class Rng {
public:
    Rng(int min, int max)
        : dist(min, max)
    {
    }

    int generate()
    {
        return dist(randomEndine);
    }

private:
    std::default_random_engine randomEndine;
    std::uniform_int_distribution<int> dist;
};

struct GenericGen {
public:
    GenericGen(const char* first, const char* second, const char* zoom, const char* key, const char* keyValue,
               const std::function<int(int)>& zoomConverter)
        : _firstStr(first)
        , _secondStr(second)
        , _zoomStr(zoom)
        , _keyStr(key)
        , _keyValueStr(keyValue)
        , _zoomConverter(zoomConverter)
        , hostRng(0, 3)
        , sRng(1, 7)
    {
    }

    std::string operator()(const TilePosition& pos)
    {
        std::string url = "http://";
        url.append(_firstStr);
        url.push_back('0' + hostRng.generate());
        url.append(_secondStr);
        url.append("&x=");
        url.append(std::to_string(pos.globalOffsetX));
        url.append("&y=");
        url.append(std::to_string(pos.globalOffsetY));
        url.push_back('&');
        url.append(_zoomStr);
        url.push_back('=');
        url.append(std::to_string(_zoomConverter(pos.zoomLevel)));
        url.push_back('&');
        url.append(_keyStr);
        url.push_back('=');
        url.append(_keyValueStr, sRng.generate());
        return url;
    }

private:
    const char* _firstStr;
    const char* _secondStr;
    const char* _zoomStr;
    const char* _keyStr;
    const char* _keyValueStr;
    std::function<int(int)> _zoomConverter;
    Rng hostRng;
    Rng sRng;
};

struct OsmGen {
    OsmGen(const char* baseUrlPrefix, const char* baseUrl, char randomCharStart, int maxChar)
        : _baseUrlPrefix(baseUrlPrefix)
        , _baseUrl(baseUrl)
        , _randomCharStart(randomCharStart)
        , _rng(0, maxChar - 1)
    {
    }

    std::string operator()(const TilePosition& pos)
    {
        std::string url = "http://";
        url.append(_baseUrlPrefix);
        url.push_back(_randomCharStart + _rng.generate());
        url.append(_baseUrl);
        url.push_back('/');
        url.append(std::to_string(pos.zoomLevel - 1));
        url.push_back('/');
        url.append(std::to_string(pos.globalOffsetX));
        url.push_back('/');
        url.append(std::to_string(pos.globalOffsetY));
        url.append(".png");
        return url;
    }

private:
    const char* _baseUrlPrefix;
    const char* _baseUrl;
    char _randomCharStart;
    Rng _rng;
};

class GoogleMapCache : public OnlineCache {
protected:
    int calcMaxTileZoom() override
    {
        return 20;
    }

    const char* createServiceName() const override
    {
        return "googlemap";
    }

    const char* createName() const override
    {
        return "Google Карта";
    }

    QString createDescription() const override
    {
        return "";
    }

    const char* createFormat() const override
    {
        return "png";
    }

    MercatorProjection::ProjectionType createProjection() const override
    {
        return MercatorProjection::SphericalMercator;
    }

    Generator createGenerator() const override
    {
        return GenericGen("mt", ".google.com/vt/lyrs/=m@169000000&hl=ru", "zoom", "s", "Galileo",
                          [](int z) { return 18 - z; });
    }
};

class GoogleLandscapeCache : public OnlineCache {
protected:
    int calcMaxTileZoom() override
    {
        return 23;
    }

    const char* createServiceName() const override
    {
        return "googlelandscape";
    }

    const char* createName() const override
    {
        return "Google Ландшафт";
    }

    QString createDescription() const override
    {
        return "";
    }

    const char* createFormat() const override
    {
        return "jpg";
    }

    MercatorProjection::ProjectionType createProjection() const override
    {
        return MercatorProjection::SphericalMercator;
    }

    Generator createGenerator() const override
    {
        return GenericGen("mt", ".google.com/vt/lyrs=t@130,r@206000000&hl=ru", "z", "s", "Galileo",
                          [](int z) { return z - 1; });
    }
};


class GoogleSatelliteCache : public OnlineCache {
protected:
    int calcMaxTileZoom() override
    {
        return 20;
    }

    const char* createServiceName() const override
    {
        return "googlesatellite";
    }

    const char* createName() const override
    {
        return "Google Спутник";
    }

    QString createDescription() const override
    {
        return "";
    }

    const char* createFormat() const override
    {
        return "jpg";
    }

    MercatorProjection::ProjectionType createProjection() const override
    {
        return MercatorProjection::SphericalMercator;
    }

    Generator createGenerator() const override
    {
        return GenericGen("khms", ".google.com/kh/v=163&src=app", "z", "s", "Galileo", [](int z) { return z - 1; });
    }
};

class YandexMapCache : public OnlineCache {
protected:
    int calcMaxTileZoom() override
    {
        return 18;
    }

    const char* createServiceName() const override
    {
        return "yandexmap";
    }

    const char* createName() const override
    {
        return "Yandex Карта";
    }

    QString createDescription() const override
    {
        return "";
    }

    const char* createFormat() const override
    {
        return "png";
    }

    MercatorProjection::ProjectionType createProjection() const override
    {
        return MercatorProjection::EllipticalMercator;
    }

    Generator createGenerator() const override
    {
        return GenericGen("vec0", ".maps.yandex.net/tiles?l=map&", "z", "g", "Gagarin", [](int z) { return z - 1; });
    }
};

class YandexNarodnayaCache : public OnlineCache {
protected:
    int calcMaxTileZoom() override
    {
        return 18;
    }

    const char* createServiceName() const override
    {
        return "yandexnarodnaya";
    }

    const char* createName() const override
    {
        return "Yandex Народная";
    }

    QString createDescription() const override
    {
        return "";
    }

    const char* createFormat() const override
    {
        return "png";
    }

    MercatorProjection::ProjectionType createProjection() const override
    {
        return MercatorProjection::EllipticalMercator;
    }

    Generator createGenerator() const override
    {
        return GenericGen("0", ".pvec.maps.yandex.net/?l=pmap&lang=ru-RU&", "z", "g", "Gagarin",
                          [](int z) { return z - 1; });
    }
};

class YandexSatelliteCache : public OnlineCache {
protected:
    int calcMaxTileZoom() override
    {
        return 18;
    }

    const char* createServiceName() const override
    {
        return "yandexsatellite";
    }

    const char* createName() const override
    {
        return "Yandex Спутник";
    }

    QString createDescription() const override
    {
        return "";
    }

    const char* createFormat() const override
    {
        return "jpg";
    }

    MercatorProjection::ProjectionType createProjection() const override
    {
        return MercatorProjection::EllipticalMercator;
    }

    Generator createGenerator() const override
    {
        return GenericGen("sat0", ".maps.yandex.net/tiles?l=sat&", "z", "g", "Gagarin", [](int z) { return z - 1; });
    }
};

class OsmBasicCache : public OnlineCache {
protected:
    int calcMaxTileZoom() override
    {
        return 19;
    }

    const char* createServiceName() const override
    {
        return "osmbasic";
    }

    const char* createName() const override
    {
        return "OpenStreetMap Basic";
    }

    QString createDescription() const override
    {
        return "";
    }

    const char* createFormat() const override
    {
        return "png";
    }

    MercatorProjection::ProjectionType createProjection() const override
    {
        return MercatorProjection::SphericalMercator;
    }

    Generator createGenerator() const override
    {
        return OsmGen("", ".tile.openstreetmap.org", 'a', 3);
    }
};

class OsmThunderforestLandscape : public OnlineCache {
protected:
    int calcMaxTileZoom() override
    {
        return 19;
    }

    const char* createServiceName() const override
    {
        return "osmthunderforestlandscape";
    }

    const char* createName() const override
    {
        return "OpenStreetMap Thunderforest Landscape";
    }

    QString createDescription() const override
    {
        return "";
    }

    const char* createFormat() const override
    {
        return "png";
    }

    MercatorProjection::ProjectionType createProjection() const override
    {
        return MercatorProjection::SphericalMercator;
    }

    Generator createGenerator() const override
    {
        return OsmGen("", ".tile.thunderforest.com/landscape", 'a', 3);
    }
};

class OsmMapQuestCache : public OnlineCache {
protected:
    int calcMaxTileZoom() override
    {
        return 19;
    }

    const char* createServiceName() const override
    {
        return "osmmapquest";
    }

    const char* createName() const override
    {
        return "OpenStreetMap MapQuest";
    }

    QString createDescription() const override
    {
        return "";
    }

    const char* createFormat() const override
    {
        return "jpg";
    }

    MercatorProjection::ProjectionType createProjection() const override
    {
        return MercatorProjection::SphericalMercator;
    }

    Generator createGenerator() const override
    {
        return OsmGen("otile", ".mqcdn.com/tiles/1.0.0/osm", '1', 4);
    }
};

class OsmMapQuestSatCache : public OnlineCache {
protected:
    int calcMaxTileZoom() override
    {
        return 19;
    }

    const char* createServiceName() const override
    {
        return "osmmapquestsat";
    }

    const char* createName() const override
    {
        return "OpenStreetMap MapQuest Sat";
    }

    QString createDescription() const override
    {
        return "";
    }

    const char* createFormat() const override
    {
        return "jpg";
    }

    MercatorProjection::ProjectionType createProjection() const override
    {
        return MercatorProjection::SphericalMercator;
    }

    Generator createGenerator() const override
    {
        return OsmGen("otile", ".mqcdn.com/tiles/1.0.0/sat", '1', 4);
    }
};

static OnlineCachePtr createCache(StaticMapType type)
{
    switch(type) {
    case StaticMapType::GoogleMap:
        return std::make_shared<GoogleMapCache>();
    case StaticMapType::GoogleLandscape:
        return std::make_shared<GoogleLandscapeCache>();
    case StaticMapType::GoogleSatellite:
        return std::make_shared<GoogleSatelliteCache>();
    case StaticMapType::YandexMap:
        return std::make_shared<YandexMapCache>();
    case StaticMapType::YandexNarodnaya:
        return std::make_shared<YandexNarodnayaCache>();
    case StaticMapType::YandexSatellite:
        return std::make_shared<YandexSatelliteCache>();
    case StaticMapType::OsmBasic:
        return std::make_shared<OsmBasicCache>();
    case StaticMapType::OsmMapQuest:
        return std::make_shared<OsmMapQuestCache>();
    case StaticMapType::OsmMapQuestSat:
        return std::make_shared<OsmMapQuestSatCache>();
    case StaticMapType::OsmThunderforestLandscape:
        return std::make_shared<OsmThunderforestLandscape>();
    }
}

OnlineCachePtr OnlineCache::create(StaticMapType type)
{
    OnlineCachePtr cache = createCache(type);;
    cache->setPath(core::Settings::instance()->mapCachePath(), cache->createServiceName());
    cache->setFormat(cache->createFormat());
    cache->_maxTileZoom = cache->calcMaxTileZoom();
    cache->_generator = cache->createGenerator();
    cache->_proj = cache->createProjection();
    cache->_name = cache->createName();
    cache->_description = cache->createDescription();
    cache->_type = type;
    return cache;
}

OnlineCache::OnlineCache()
    : DiskCache(core::Settings::instance()->mapCachePath())
{
}

OnlineCache::~OnlineCache()
{
}

void OnlineCache::setBasePath(const QString& path)
{
    setPath(path, createServiceName());
}

bool OnlineCache::isBuiltIn() const
{
    return true;
}

std::string OnlineCache::generateTileUrl(const TilePosition& pos)
{
    return _generator(pos);
}

const MercatorProjection& OnlineCache::projection() const
{
    return _proj;
}

int OnlineCache::maxTileZoom() const
{
    return _maxTileZoom;
}

bool OnlineCache::hasOnlineTiles() const
{
    return true;
}

const QString& OnlineCache::description() const
{
    return _description;
}

const QString& OnlineCache::name() const
{
    return _name;
}
}
}
}
