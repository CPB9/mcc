/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#pragma once

#include <QVector>
#include <QMetaType>

namespace mcc {
    namespace ui {
        namespace core{

            enum class TrailMode { All, Distance, Time, None };

            bool doubleEq(double a, double b, unsigned int maxUlps = 4);

            struct LatLon
            {
                double latitude;
                double longitude;

                LatLon()
                    : latitude(0.0)
                    , longitude(0.0)
                {
                }

                LatLon(double latitude, double longitude)
                    : latitude(latitude)
                    , longitude(longitude)
                {
                }

                LatLon(const LatLon& position)
                    : LatLon(position.latitude, position.longitude)
                {
                }
            };

            struct GeoBox
            {
                LatLon topLeft;
                LatLon bottomRight;

                GeoBox()
                {
                }

                explicit GeoBox(const LatLon& single)
                    : topLeft(single)
                    , bottomRight(single)
                {
                }

                GeoBox(const LatLon& topLeft, const LatLon& bottomRight)
                    : topLeft(topLeft)
                    , bottomRight(bottomRight)
                {
                }

                LatLon center() const
                {
                    double dlat = (topLeft.latitude - bottomRight.latitude) / 2;
                    double dlon = (bottomRight.longitude - topLeft.longitude) / 2;
                    return LatLon(bottomRight.latitude + dlat, topLeft.longitude + dlon);
                }

                bool isPoint() const
                {
                    return topLeft.latitude == bottomRight.latitude && topLeft.longitude == bottomRight.longitude;
                }
            };

            struct GeoPosition
            {
                double latitude;
                double longitude;
                double altitude;

                GeoPosition()
                    : latitude(0.0)
                    , longitude(0.0)
                    , altitude(0.0)
                {
                }

                GeoPosition(double latitude, double longitude, double altitude)
                    : latitude(latitude)
                    , longitude(longitude)
                    , altitude(altitude)
                {
                }

                GeoPosition(const GeoPosition& position)
                    : GeoPosition(position.latitude, position.longitude, position.altitude)
                {
                }

                LatLon latLon() const
                {
                    return LatLon(latitude, longitude);
                }

                bool isValid() const
                {
                    return latitude > -180.0 && latitude < 180.0
                        && longitude > -90.0 && longitude < 90.0;
                }
            };

            bool operator ==(const GeoPosition& left, const GeoPosition& right);
            bool operator !=(const GeoPosition& left, const GeoPosition& right);

            struct GeoOrientation
            {
                double heading;
                double pitch;
                double roll;

                GeoOrientation()
                    : heading(0.0)
                    , pitch(0.0)
                    , roll(0.0)
                {
                }

                GeoOrientation(double heading, double pitch, double roll)
                    : heading(heading)
                    , pitch(pitch)
                    , roll(roll)
                {
                }

                GeoOrientation(const GeoOrientation& orientation)
                    : GeoOrientation(orientation.heading, orientation.pitch, orientation.roll)
                {
                }
            };

            bool operator ==(const GeoOrientation& left, const GeoOrientation& right);
            bool operator !=(const GeoOrientation& left, const GeoOrientation& right);

            enum class WaypointType
            {
                Normal        = 0,   //! Обычный вейпоинт
                Home          = 1,   //! Дом
                Turn          = 2,   //! Разворот
                ByAltitude    = 4,   //! По достижению высоты
                Target        = 8,   //! Цель
                SwitchRoute   = 16,  //! Переключение на другой маршрут
                ReturnToRoute = 32,  //! Возвращение на маршрут
                Waiting       = 64,  //! Ожидание
                Interpolation = 128, //! Интерполяция
                ByHeading     = 256, //! Управление по курсу
            };

            Q_DECLARE_FLAGS(WaypointFlags, WaypointType);

            struct Waypoint
            {
                Waypoint()
                    : Waypoint(-1, 0.0, 0.0, 0.0, 0.0)
                {
                }

                Waypoint(double latitude, double longitude, double altitude, double speed)
                    : Waypoint(-1, latitude, longitude, altitude, speed)
                {
                }

                Waypoint(int index, double latitude, double longitude, double altitude, double speed)
                {
                    this->index    = index;
                    this->position = GeoPosition(latitude, longitude, altitude);
                    this->speed    = speed;
                    this->flags = WaypointType::Normal;
                }

                Waypoint(int index, double latitude, double longitude, double altitude, double speed, WaypointType type)
                {
                    this->index    = index;
                    this->position = GeoPosition(latitude, longitude, altitude);
                    this->speed    = speed;
                    this->flags    = type;
                }

                Waypoint(const core::LatLon& latLon, double height, double speed)
                    : Waypoint(-1, latLon.latitude, latLon.longitude, height, speed)
                {

                }

                Waypoint(const Waypoint& other)
                {
                    this->index    = other.index;
                    this->position = other.position;
                    this->speed    = other.speed;
                    this->flags    = other.flags;
                }

                LatLon latLon() const
                {
                    return position.latLon();
                }

                int           index;         //! Индекс вейпоинта
                GeoPosition   position;      //! Положение в пространстве
                double        speed;         //! Скорость
                WaypointFlags flags;         //! Флаги вейпоинта
            };

            bool operator ==(const Waypoint& left, const Waypoint& right);
            bool operator !=(const Waypoint& left, const Waypoint& right);

            typedef QVector<Waypoint> WaypointsList;
        }
    }
}

Q_DECLARE_METATYPE(mcc::ui::core::GeoPosition);
Q_DECLARE_METATYPE(mcc::ui::core::GeoOrientation);
