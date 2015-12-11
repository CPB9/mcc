/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#pragma once

#include "mcc/misc/Option.h"

#include <QString>
#include <QLineEdit>
#include <QValidator>
#include <QDoubleValidator>

#include <utility>

namespace mcc {
namespace ui {
namespace core {

static const QChar degreeChar(0260);

enum class CoordinateSystem {
    WGS84,
    SK42
};

enum class CoordinateFormat {
    Degrees,
    DegreesMinutes,
    DegreesMinutesSeconds
};

class LatLonEdit;
class CoordinatePrinter;

class PrinterDegreesValidator : public QValidator
{
public:
    PrinterDegreesValidator(const CoordinatePrinter &printer, double minDegrees, double maxDegrees);

    virtual State validate(QString &input, int &pos) const;

private:
    const CoordinatePrinter &_printer;
    double _minDegrees;
    double _maxDegrees;
};

class CoordinatePrinter {
public:
    CoordinatePrinter();

    double fromWgs84Lat(double lat, double lon) const;
    double fromWgs84Lon(double lat, double lon) const;
    double toWgs84Lat(double lat, double lon) const;
    double toWgs84Lon(double lat, double lon) const;

    QString print(double lat, double lon, const QString& arg) const;
    QString printLat(double lat, double lon) const;
    QString printLon(double lat, double lon) const;
    QString printLatExternal(double lat, double lon) const;
    QString printLonExternal(double lat, double lon) const;
    QString printValue(double value, int prec = 5) const;
    misc::Option<double> parse(const QString& line) const;

    void setTargetSystem(CoordinateSystem system);
    void setFormat(CoordinateFormat format);
    CoordinateFormat format() const;
    CoordinateSystem system() const;

    LatLonEdit *latitudeEditor(QWidget *parent) const;
    LatLonEdit *longitudeEditor(QWidget *parent) const;
    QValidator *latitudeValidator() const;
    QValidator *longitudeValidator() const;
    QString latitudeInputMask() const;
    QString longitudeInputMask() const;
private:
    misc::Option<double> parseDegrees(const QString& line) const;
    misc::Option<double> parseDegreesMinutes(const QString& line) const;
    misc::Option<double> parseDegreesMinutesSeconds(const QString& line) const;
    QString printValueExternal(double value) const;
    QString externalizeString(const QString& value) const;
    CoordinateFormat _format;
    CoordinateSystem _system;
};

inline CoordinateFormat CoordinatePrinter::format() const
{
    return _format;
}

inline CoordinateSystem CoordinatePrinter::system() const
{
    return _system;
}
}
}
}
