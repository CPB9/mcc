/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#pragma once

#include <QObject>
#include <QVector>
#include <QPen>

#include "mcc/ui/core/Structs.h"

namespace mcc {
    namespace ui {
        namespace core {

            class Route : public QObject
            {
                Q_OBJECT

            public:
                enum class FileFormat
                {
                    Json,
                };

                Route(const QString& name, int id, bool isBuffer = false);

                Route(const Route& other);
                ~Route();
                int                  id() const;
                const Waypoint&      waypointAt(int idx) const;
                const WaypointsList& waypointsList() const;
                int                  waypointsCount() const;
                bool                 ring() const;
                bool                 visible() const;
                bool                 showDetails() const;

                const QPen&          pen() const;
                const QColor&        activePointColor() const;
                const QColor&        inactivePointColor() const;

                const QString&       name() const;
                const QString&       crc() const;

                GeoBox computeBoundindBox() const;
                void copyFrom(Route* other);

                Route*               buffer() const;
                void                 resetBuffer();
                void                 reverse();

                void                 save(const QString& path, FileFormat fmt) const;
                void                 load(const QString& path);
            public slots:
                void clear();
                bool addWaypoint(const Waypoint& waypoint);
                void addWaypointNoEmit(const Waypoint& waypoint);
                bool setWaypoint(const Waypoint& waypoint, int idx);
                bool insertWaypoint(const Waypoint& waypoint, int afterIndex = -1);
                bool removeWaypoint(int index);

                bool moveWaypointUp(int index);
                bool moveWaypointDown(int index);

                void setWaypoints(const WaypointsList& waypoints, const QString& crc);

                void setRing(bool isRing);

                void setVisible(bool visible);
                void setStyle(const QPen& pen, const QColor& activePointColor, const QColor& inactivePointColor, bool showDetails);
            signals:
                void routeChanged();
                void ringModeChanged(bool isRing);

                void waypointInserted(const Waypoint& waypoint, int index);
                void waypointRemoved(int index);
                void waypointMoved(int oldIndex, int newIndex);
                void waypointChanged(const Waypoint& waypoint, int index);

                void visibilityChanged(bool visible);
                void styleChagned(const QPen& brush, const QColor& activePointColor, const QColor& inactivePointColor, bool showDetails);
            private:
                void saveJson(const QString& path) const;

                WaypointsList _points;
                bool          _isRing;
                bool          _showDetails;

                int           _id;
                bool          _isVisible;
                QPen          _pen;
                QColor        _activePointColor;
                QColor        _inactivePointColor;

                QString       _name;
                QString       _crc;

                Route*        _buffer;
            };

        }
    }
}

