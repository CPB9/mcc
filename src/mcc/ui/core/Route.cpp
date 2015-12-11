/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "mcc/ui/core/Route.h"
#include <QColor>
#include <QDebug>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QFile>
#include <QTextStream>
#include <QJsonParseError>

namespace mcc {
    namespace ui {
        namespace core {

            Route::Route(const QString& name, int id, bool isBuffer)
                : QObject()
                , _isRing(true)
                , _showDetails(true)
                , _id(id)
                , _isVisible(true)
                , _name(name)
                , _buffer(nullptr)
            {
                _pen.setColor(QColor(Qt::yellow));
                _pen.setWidth(2);

                _activePointColor = Qt::red;
                _inactivePointColor = Qt::blue;

                if (!isBuffer)
                    _buffer = new Route(name + "_buffer", id, true);
            }

            Route::~Route()
            {
                delete _buffer;
            }

            Route::Route(const Route& other) : QObject()
            {
                Q_UNUSED(other);
            }

            void Route::clear()
            {
                _points.clear();
                emit(((routeChanged())));
            }

            const mcc::ui::core::Waypoint& Route::waypointAt(int idx) const
            {
                Q_ASSERT(idx < _points.size());

                return _points[idx];
            }

            const mcc::ui::core::WaypointsList& Route::waypointsList() const
            {
                return _points;
            }

            int Route::waypointsCount() const
            {
                return _points.count();
            }

            void Route::addWaypointNoEmit(const Waypoint& waypoint)
            {
                _points.append(waypoint);
            }

            bool Route::addWaypoint(const Waypoint& waypoint)
            {
                _points.append(waypoint);
                emit((waypointInserted(waypoint, _points.count() - 1)));
                return true;
            }

            bool Route::setWaypoint(const Waypoint& waypoint, int idx)
            {
                if (idx >= _points.size())
                    return false;

                if (_points[idx] != waypoint)
                {
                    _points[idx] = waypoint;
                    emit(((((waypointChanged(waypoint, idx))))));
                }

                return true;
            }

            bool Route::insertWaypoint(const Waypoint& waypoint, int afterIndex /*= -1*/)
            {
                if (afterIndex == -1)
                    afterIndex = _points.count();

                _points.insert(afterIndex, waypoint);
                emit((((waypointInserted(waypoint, afterIndex)))));

                return true;
            }

            bool Route::removeWaypoint(int index)
            {
                if (index < 0 || index >= waypointsCount())
                    return false;

                _points.remove(index);

                emit(waypointRemoved(index));

                return true;
            }

            bool Route::moveWaypointUp(int index)
            {
                if (index <= 0)
                    return false;

                int oldIndex = index;
                int newIndex = index - 1;

                auto wp = _points.at(oldIndex);
                _points.remove(oldIndex);
                _points.insert(newIndex, wp);

                emit(waypointMoved(oldIndex, newIndex));

                return true;
            }

            bool Route::moveWaypointDown(int index)
            {
                if (index >= _points.count() - 1)
                    return false;

                int oldIndex = index;
                int newIndex = index + 1;

                auto wp = _points.at(oldIndex);
                _points.remove(oldIndex);
                _points.insert(newIndex, wp);

                emit(waypointMoved(oldIndex, newIndex));

                return true;
            }

            void Route::setWaypoints(const WaypointsList& waypoints, const QString& crc)
            {
                if (_points == waypoints && _crc == crc)
                    return;

                _crc = crc;

                _points = waypoints;
                emit(routeChanged());
            }

            bool Route::ring() const
            {
                return _isRing;
            }

            void Route::setRing(bool isRing)
            {
                _isRing = isRing;
                emit ringModeChanged(isRing);
            }

            bool Route::visible() const
            {
                return _isVisible;
            }

            void Route::setVisible(bool visible)
            {
                if (_isVisible != visible)
                {
                    _isVisible = visible;
                    emit visibilityChanged(_isVisible);
                }
            }


            const QPen& Route::pen() const
            {
                return _pen;
            }


            const QColor& Route::activePointColor() const
            {
                return _activePointColor;
            }

            const QColor& Route::inactivePointColor() const
            {
                return _inactivePointColor;
            }

            const QString& Route::name() const
            {
                return _name;
            }

            int Route::id() const
            {
                return _id;
            }

            void Route::setStyle(const QPen& pen, const QColor& activePointColor, const QColor& inactivePointColor, bool showDetails)
            {
                _pen = pen;
                _activePointColor = activePointColor;
                _inactivePointColor = inactivePointColor;
                _showDetails = showDetails;

                emit styleChagned(_pen, _activePointColor, _inactivePointColor, _showDetails);
            }

            void Route::copyFrom(Route* other)
            {
                _id = other->id();
                setRing(other->ring());
                setWaypoints(other->waypointsList(), other->crc());
            }

            const QString& Route::crc() const
            {
                return _crc;
            }

            GeoBox Route::computeBoundindBox() const
            {
                if (_points.size() == 0)
                    return GeoBox();
                else if (_points.size() == 1)
                    return GeoBox(_points[0].latLon());

                GeoBox bbox(_points[0].latLon());
                for (auto it = (_points.begin() + 1); it < _points.end(); it++)
                {
                    double lat = it->position.latitude;
                    if (lat > bbox.topLeft.latitude)
                        bbox.topLeft.latitude = lat;
                    else if (lat < bbox.bottomRight.latitude)
                        bbox.bottomRight.latitude = lat;
                    double lon = it->position.longitude;
                    if (lon < bbox.topLeft.longitude)
                        bbox.topLeft.longitude = lon;
                    else if (lon > bbox.bottomRight.longitude)
                        bbox.bottomRight.longitude = lon;
                }
                return bbox;
            }

            Route* Route::buffer() const
            {
                return _buffer;
            }

            void Route::resetBuffer()
            {
                _buffer->copyFrom(this);
            }

            void Route::save(const QString& path, FileFormat fmt) const
            {
                switch (fmt)
                {
                case FileFormat::Json:
                    saveJson(path);
                }
            }

            void Route::load(const QString& path)
            {
                QFile file(path);
                if (!file.open(QIODevice::ReadOnly))
                {
                    qDebug() << "Unable to open " << path;
                    return;
                }

                QJsonParseError error;
                QJsonDocument doc = QJsonDocument::fromJson(file.readAll(), &error);

                if (error.error != QJsonParseError::NoError)
                {
                    qDebug() << "Wrong json document " << path;
                    return;
                }

                QJsonObject routeObj = doc.object();

                if (!routeObj.contains("waypoints"))
                {
                    qDebug() << "Wrong json document " << path;
                    return;
                }

                if (!routeObj["waypoints"].isArray())
                {
                    qDebug() << "Wrong json document " << path;
                    return;
                }

                QJsonArray waypointsObj = routeObj["waypoints"].toArray();

                WaypointsList wps;
                int index = 0;
                for (const auto& wp : waypointsObj)
                {
                    QJsonObject wpObj = wp.toObject();

                    double latitude = wpObj["latitude"].toDouble();
                    double longitude = wpObj["longitude"].toDouble();
                    double altitude = wpObj["altitude"].toDouble();
                    double speed = wpObj["speed"].toDouble();
                    quint32 flags = 0;

                    wps.append(Waypoint(index++, latitude, longitude, altitude, speed, static_cast<WaypointType>(flags)));
                }

                bool isRing = routeObj["ring"].toBool();

                setWaypoints(wps, 0);
                setRing(isRing);
            }

            void Route::saveJson(const QString& path) const
            {
                QJsonArray waypoints;

                for (const auto& wp : _points)
                {
                    QJsonObject wpObj;
                    wpObj["latitude"] = wp.position.latitude;
                    wpObj["longitude"] = wp.position.longitude;
                    wpObj["altitude"] = wp.position.altitude;

                    wpObj["speed"] = wp.speed;

                    waypoints.append(wpObj);
                }

                QJsonObject routeObj;
                routeObj["waypoints"] = waypoints;
                routeObj["ring"] = this->ring();

                QFile outputFile(path);
                outputFile.open(QIODevice::WriteOnly);

                if (!outputFile.isOpen())
                {
                    qDebug() << "Unable to open " << path;
                    return;
                }

                QJsonDocument doc;
                doc.setObject(routeObj);

                QTextStream outStream(&outputFile);
                outStream << doc.toJson(QJsonDocument::Indented);

                outputFile.close();
            }

            void Route::reverse()
            {
                std::reverse(_points.begin(), _points.end());
                emit routeChanged();
            }

            bool Route::showDetails() const
            {
                return _showDetails;
            }


        }
    }
}
