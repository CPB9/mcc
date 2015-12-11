/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "mcc/ui/core/Settings.h"

#include <QSettings>
#include <QStandardPaths>
#include <QByteArray>

namespace mcc {
namespace ui {
namespace core {

Settings::Settings()
    : _settings(organizationName(), applicationName())
    , _mapAnimation(true)
{
    auto variant = _settings.value("map/animation");
    if (variant.isValid())
        _mapAnimation = variant.toBool();

    auto inverted = _settings.value("pfd/inverted");
    if (inverted.isValid())
        _invertedPfd = inverted.toBool();
}

QVariant Settings::get(const QString& key, const QVariant& defaultValue) const
{
    return _settings.value(key, defaultValue);
}

void Settings::set(const QString& key, const QVariant& value)
{
    _settings.setValue(key, value);
}

QString Settings::mapCachePath() const
{
    auto variant = _settings.value("map/cachePath");
    if (variant.isValid()) {
        return variant.toString();
    } else {
        return QStandardPaths::writableLocation(QStandardPaths::CacheLocation);
    }
}

void Settings::setMapCachePath(const QString& path)
{
    _settings.setValue("map/cachePath", path);
    emit mapCachePathChanged(path);
}

bool Settings::mapAnimation() const
{
    return _mapAnimation;
}

void Settings::setMapAnimation(bool animation)
{
    _mapAnimation = animation;
    _settings.setValue("map/animation", animation);
    emit mapAnimationChanged(animation);
}

const char* Settings::organizationName()
{
    return "Applmech";
}

const char* Settings::applicationName()
{
    return "Mcc";
}

Settings* Settings::instance()
{
    static Settings staticInstance; // в c++11 все окай
    return &staticInstance;
}

void Settings::resetMapCachePath()
{
    _settings.remove("map/cachePath");
    emit mapCachePathChanged(mapCachePath());
}

CoordinateFormat Settings::coordFormat() const
{
    auto variant = _settings.value("global/coordFormat");
    if (variant.isValid()) {
        return static_cast<CoordinateFormat>(variant.toInt());
    } else {
        return CoordinateFormat::Degrees;
    }
}

void Settings::setCoordFormat(CoordinateFormat format)
{
    _settings.setValue("global/coordFormat", static_cast<int>(format));
}

CoordinateSystem Settings::coordSystem() const
{
    auto variant = _settings.value("global/coordSystem");
    if (variant.isValid()) {
        return static_cast<CoordinateSystem>(variant.toInt());
    } else {
        return CoordinateSystem::WGS84;
    }
}

void Settings::setCoordSystem(CoordinateSystem system)
{
    _settings.setValue("global/coordSystem", static_cast<int>(system));
}

LatLon Settings::latLon() const
{
    auto latitude = _settings.value("map/latitude");
    auto longitude = _settings.value("map/longitude");

    if (latitude.isValid() && longitude.isValid()) {
        return LatLon(latitude.toDouble(), longitude.toDouble());
    }

    return LatLon();
}

void Settings::setLatLon(const LatLon& latLon)
{
    _settings.setValue("map/latitude", latLon.latitude);
    _settings.setValue("map/longitude", latLon.longitude);
}

int Settings::zoom() const
{
    auto zoom = _settings.value("map/zoom");

    if (zoom.isValid()) {
        return zoom.toInt();
    }

    return 1;
}

void Settings::setZoom(int zoom)
{
    _settings.setValue("map/zoom", zoom);
}

WindowGeometry Settings::windowGeometry() const
{
    auto pos = _settings.value("window/geometry/pos");
    auto size = _settings.value("window/geometry/size");
    auto maximized = _settings.value("window/geometry/maximized");

    if (pos.isValid() && size.isValid() && maximized.isValid()) {
        WindowGeometry geometry;
        geometry.position = pos.toPoint();
        geometry.size = size.toSize();
        geometry.maximized = maximized.toBool();
        return geometry;
    }

    return WindowGeometry();
}

void Settings::setWindowGeometry(const WindowGeometry& geometry)
{
    _settings.setValue("window/geometry/pos", geometry.position);
    _settings.setValue("window/geometry/size", geometry.size);
    _settings.setValue("window/geometry/maximized", geometry.maximized);
}

QByteArray Settings::windowState() const
{
    auto state = _settings.value("window/state");

    if (state.isValid()) {
        return state.toByteArray();
    }

    return QByteArray();
}

void Settings::setWindowState(const QByteArray& state)
{
    _settings.setValue("window/state", state);
}

QStringList Settings::tabifiedTools() const
{
    auto state = _settings.value("window/tabified_tools");

    if (state.isValid()) {
        return state.toStringList();
    }

    return QStringList();
}

void Settings::setTabifiedTools(const QStringList& tools)
{
    _settings.setValue("window/tabified_tools", tools);
}

void Settings::restoreDefaults()
{
    _settings.clear();
}

bool Settings::invertedPfd() const
{
    return _invertedPfd;
}

void Settings::setInvertedPfd(bool inverted)
{
    _invertedPfd = inverted;
    _settings.setValue("pfd/inverted", inverted);
    emit invertedPfdChanged(inverted);
}

QString Settings::currentQmlPath() const
{
    return get("ide/currentQmlPath", "").toString();
}

void Settings::setCurrentQmlPath(const QString& path)
{
    set("ide/currentQmlPath", path);
}

void Settings::setStaticMapType(map::StaticMapType type)
{
    set("map/staticMapType", uint(type));
}

map::StaticMapType Settings::staticMapType() const
{
    return (map::StaticMapType)get("map/staticMapType").toUInt();
}

mcc::ui::core::SerialSettings Settings::gpsSettings() const
{
    auto port = _settings.value("gps/settings/port");
    auto speed = _settings.value("gps/settings/speed");

    if (port.isValid() && speed.isValid()) {
        return SerialSettings(port.toString(), speed.toInt());
    }

    return SerialSettings();
}

void Settings::setGpsSettings(const SerialSettings& settings)
{
    _settings.setValue("gps/settings/port", settings.first);
    _settings.setValue("gps/settings/speed", settings.second);

    emit gpsSettingsChanged(settings);
}

mcc::ui::core::LatLon Settings::homePosition() const
{
    auto latitude = _settings.value("gps/home/latitude");
    auto longitude = _settings.value("gps/home/longitude");

    if (latitude.isValid() && longitude.isValid()) {
        return LatLon(latitude.toDouble(), longitude.toDouble());
    }

    return LatLon();
}

void Settings::setHomePosition(const LatLon& latLon)
{
    _settings.setValue("gps/home/latitude", latLon.latitude);
    _settings.setValue("gps/home/longitude", latLon.longitude);
}

MapMode Settings::mapMode() const
{
    return (MapMode)get("map/mapMode").toUInt();
}

void Settings::setMapMode(MapMode mode)
{
    set("map/mapMode", uint(mode));
}

QByteArray Settings::mapStackState() const
{
    return get("map/stackState").toByteArray();
}

void Settings::setMapStackState(const QByteArray& state)
{
    set("map/stackState", state);
}

QString Settings::lastStackPath() const
{
    return get("map/lastStackPath").toString();
}

void Settings::setLastStackPath(const QString& path)
{
    set("map/lastStackPath", path);
}
}
}
}
