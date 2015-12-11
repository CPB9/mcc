/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#pragma once

#include <QString>
#include <QSettings>
#include <QPair>

#include "mcc/ui/core/CoordinatePrinter.h"
#include "mcc/ui/core/Structs.h"
#include "mcc/ui/map/StaticMapType.h"

class QByteArray;

namespace mcc {
namespace ui {
namespace core {

struct WindowGeometry {
    QPoint position;
    QSize size;
    bool maximized;

    WindowGeometry()
    {
        maximized = false;
    }
};

enum class MapMode {
    Online,
    Offline,
    Stack
};

typedef QPair<QString, int> SerialSettings;

class Settings : public QObject {
    Q_OBJECT

signals:
    void mapCachePathChanged(const QString& path);
    void mapAnimationChanged(bool animation);
    void invertedPfdChanged(bool inverted);

    void gpsSettingsChanged(const SerialSettings& settings);

public:
    static Settings* instance();

    static const char* organizationName();
    static const char* applicationName();

    QVariant get(const QString& key, const QVariant& defaultValue = QVariant()) const;
    void set(const QString& key, const QVariant& variant);

    QString mapCachePath() const;
    void setMapCachePath(const QString& path);
    void resetMapCachePath();

    QString currentQmlPath() const;
    void setCurrentQmlPath(const QString& path);

    bool mapAnimation() const;
    void setMapAnimation(bool animation);

    bool invertedPfd() const;
    void setInvertedPfd(bool inverted);

    CoordinateFormat coordFormat() const;
    void setCoordFormat(CoordinateFormat format);

    CoordinateSystem coordSystem() const;
    void setCoordSystem(CoordinateSystem system);

    LatLon latLon() const;
    void setLatLon(const LatLon& latLon);

    int zoom() const;
    void setZoom(int zoom);

    WindowGeometry windowGeometry() const;
    void setWindowGeometry(const WindowGeometry& geometry);

    QByteArray windowState() const;
    void setWindowState(const QByteArray& state);

    QStringList tabifiedTools() const;
    void setTabifiedTools(const QStringList& tools);

    map::StaticMapType staticMapType() const;
    void setStaticMapType(map::StaticMapType type);

    SerialSettings gpsSettings() const;
    void setGpsSettings(const SerialSettings& settings);

    LatLon homePosition() const;
    void setHomePosition(const LatLon& latLon);

    MapMode mapMode() const;
    void setMapMode(MapMode mode);

    QByteArray mapStackState() const;
    void setMapStackState(const QByteArray& state);

    QString lastStackPath() const;
    void setLastStackPath(const QString& path);

    Settings(const Settings& other) = delete;
    Settings(Settings&& other) = delete;
    Settings& operator=(const Settings& other) = delete;
    Settings& operator=(Settings&& other) = delete;

    void restoreDefaults();

private:
    Settings();

    QSettings _settings;
    bool _mapAnimation;
    bool _invertedPfd;
};
}
}
}
