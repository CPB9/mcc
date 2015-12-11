/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#pragma once

#include <QObject>

#include "mcc/ui/core/Structs.h"

#include <QSerialPort>

#include <nmea/nmea.h>
#include "mcc/ui/core/Settings.h"

namespace mcc {
    namespace ui {
        namespace core {

            class PositionProvider : public QObject
            {
                Q_OBJECT
            public:
                PositionProvider(QObject* parent = 0)
                    : QObject(parent)
                {
                    _stream = new QSerialPort(parent);

                    connect(_stream, SIGNAL(error(QSerialPort::SerialPortError)), this, SLOT(error(QSerialPort::SerialPortError)));
                    connect(_stream, &QSerialPort::readyRead, this, &PositionProvider::readyRead);

                    nmea_parser_init(&_parser);
                    nmea_zero_INFO(&_info);

                    connect(Settings::instance(), &Settings::gpsSettingsChanged, this, &PositionProvider::setGpsSettings);

                    auto settings = Settings::instance()->gpsSettings();
                    if (!settings.first.isEmpty())
                    {
                        setSerialPort(settings.first);
                    }

                    auto pos = Settings::instance()->homePosition();
                    setPosition(pos);
                }

                ~PositionProvider()
                {
                    nmea_parser_destroy(&_parser);
                }

                LatLon position() const { return _currentPosition; }

                void enableGps()
                {
                    _stream->open(QIODevice::ReadOnly);

                    emit connectionStateChanged(true, "");
                }

                void disableGps()
                {
                    _stream->close();

                    emit connectionStateChanged(false, "");
                }

                void setSerialPort(const QString& source)
                {
                    _stream->setPortName(source);
                }

                void setPosition(const LatLon& position)
                {
                    _currentPosition = position;

                    emit positionUpdated(_currentPosition);

                    Settings::instance()->setHomePosition(position);
                }

                bool connectionState()
                {
                    return _stream->isOpen();
                }

            signals:
                void positionUpdated(const LatLon& position);
                void connectionStateChanged(bool state, const QString& errorString);
                void satStateUpdated(int inUse, int inView, const QString& fix);

            private slots:
                void updateTimeout()
                {
                    qDebug() << "GPS Update timed out";
                }

                void error(QSerialPort::SerialPortError error)
                {
                    if (error == QSerialPort::NoError)
                        emit connectionStateChanged(true, "");

                    emit connectionStateChanged(false, _stream->errorString());
                }

                void readyRead()
                {
                    QString line = _stream->readLine();
                    if (line.length() == -1)
                        return;

                    if (!line.startsWith('$'))
                        return;

                    std::string buffer(line.toStdString().data(), line.length());

                    int parse_result = nmea_parse(&_parser, buffer.data(), buffer.size(), &_info);
                    if (parse_result == 0)
                        return;

                    nmeaPOS pos;
                    nmea_info2pos(&_info, &pos);

                    QString fix;
                    switch (_info.fix)
                    {
                    case 1:
                        fix = "Fix not available";
                        break;
                    case 2:
                        fix = "2D fix";
                        break;
                    case 3:
                        fix = "3D fix";
                        break;
                    }

                    emit satStateUpdated(_info.satinfo.inuse, _info.satinfo.inview, fix);

                    if (_info.sig)
                    {
                        LatLon ln(nmea_radian2degree(pos.lat), nmea_radian2degree(pos.lon));
                        setPosition(ln);
                    }
                }

                void setGpsSettings(const mcc::ui::core::SerialSettings& settings)
                {
                    setSerialPort(settings.first);
                }

            private:
                LatLon _currentPosition;

                QSerialPort* _stream;
                nmeaPARSER _parser;
                nmeaINFO _info;
            };

        }
    }
}

