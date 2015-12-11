/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "mcc/ui/core/CoordinatePrinter.h"
#include "mcc/ui/core/LatLonEdit.h"

#include <QtMath>
#include <QDebug>

#include <cmath>

namespace mcc {
namespace ui {
namespace core {

//TODO: refact

static double ro = 206264.8062; // Число угловых секунд в радиане

// Эллипсоид Красовского
static double aP = 6378245; // Большая полуось
static double alP = 1 / 298.3; // Сжатие
static double e2P = 2 * alP - std::pow(alP, 2); // Квадрат эксцентриситета

// Эллипсоид WGS84 (GRS80, эти два эллипсоида сходны по большинству параметров)
static double aW = 6378137; //Большая полуось
static double alW = 1 / 298.257223563; //Сжатие
static double e2W = 2 * alW - std::pow(alW, 2); // Квадрат эксцентриситета

// Вспомогательные значения для преобразования эллипсоидов
static double a = (aP + aW) / 2;
static double e2 = (e2P + e2W) / 2;
static double da = aW - aP;
static double de2 = e2W - e2P;

// Линейные элементы трансформирования, в метрах
static double dx = 23.92;
static double dy = -141.27;
static double dz = -80.9;

// Угловые элементы трансформирования, в секундах
static double wx = 0;
static double wy = 0;
static double wz = 0;

// Дифференциальное различие масштабов
static double ms = 0;

static double dLat(double lat, double lon, double height)
{
    double b = lat * M_PI / 180;
    double l = lon * M_PI / 180;
    double sb = std::sin(b);
    double cb = std::cos(b);
    double c2b = std::cos(2 * b);
    double sl = std::sin(l);
    double cl = std::cos(l);
    double sb2 = std::pow(sb, 2);
    double e2sb21 = 1 - e2 * sb2;
    double m = a * (1 - e2) / std::pow(e2sb21, 1.5);
    double n = a * std::pow(e2sb21, -0.5);
    double e2c2b1 = (1 + e2 * c2b);
    double e2sbcb = e2 * sb * cb;
    return ro / (m + height) * (n / a * e2sbcb * da + (std::pow(n, 2) / std::pow(a, 2) + 1) * n * sb * cb * de2 / 2
                                - (dx * cl + dy * sl) * sb + dz * cb) - wx * sl * e2c2b1 + wy * cl * e2c2b1
           - ro * ms * e2sbcb;
}

static double dLon(double lat, double lon, double height)
{
    double b = lat * M_PI / 180;
    double l = lon * M_PI / 180;
    double sb = std::sin(b);
    double cb = std::cos(b);
    double sl = std::sin(l);
    double cl = std::cos(l);
    double n = a * std::pow((1 - e2 * std::pow(sb, 2)), -0.5);
    return ro / ((n + height) * cb) * (-dx * sl + dy * cl) + std::tan(b) * (1 - e2) * (wx * cl + wy * sl) - wz;
}

// double WGS84Alt(double Bd, double Ld, double H)
// {
//     double B, L, N, dH;
//     B = Bd * Pi / 180;
//     L = Ld * Pi / 180;
//     N = a * std::pow((1 - e2 * std::pow(std::sin(B), 2)), -0.5);
//     dH = -a / N * da + N * std::pow(std::sin(B), 2) * de2 / 2 + (dx * std::cos(L) + dy * std::sin(L)) * std::cos(B)
//          + dz * std::sin(B) - N * e2 * std::sin(B) * std::cos(B) * (wx / ro * std::sin(L) - wy / ro * std::cos(L))
//          + (std::pow(a, 2) / N + H) * ms;
//     return H + dH;
// }

static double wgs84ToSk42Lat(double lat, double lon, double height)
{
    return lat - dLat(lat, lon, height) / 3600;
}

static double sk42ToWgs84Lat(double lat, double lon, double height)
{
    return lat + dLat(lat, lon, height) / 3600;
}

static double wgs84ToSk42Lon(double lat, double lon, double height)
{
    return lon - dLon(lat, lon, height) / 3600;
}

static double sk42ToWgs84Lon(double lat, double lon, double height)
{
    return lon + dLon(lat, lon, height) / 3600;
}

PrinterDegreesValidator::PrinterDegreesValidator(const CoordinatePrinter &printer, double minDegrees, double maxDegrees)
        : QValidator(), _printer(printer), _minDegrees(minDegrees), _maxDegrees(maxDegrees)
{
}

QValidator::State PrinterDegreesValidator::validate(QString& input, int& pos) const
{
    Q_UNUSED(pos);
    misc::Option<double> value(_printer.parse(input));
    if (value.isSome())
    {
        double d(value.unwrap());
        return (d >= _minDegrees && d <= _maxDegrees) ? QValidator::Acceptable : QValidator::Invalid;
    }
    return QValidator::Invalid;
}

CoordinatePrinter::CoordinatePrinter()
    : _format(CoordinateFormat::Degrees)
    , _system(CoordinateSystem::WGS84)
{
}

double CoordinatePrinter::fromWgs84Lat(double lat, double lon) const
{
    double value = lat;
    if (_system == CoordinateSystem::SK42) {
        value = wgs84ToSk42Lat(lat, lon, 0);
    }
    return value;
}

double CoordinatePrinter::fromWgs84Lon(double lat, double lon) const
{
    double value = lon;
    if (_system == CoordinateSystem::SK42) {
        value = wgs84ToSk42Lon(lat, lon, 0);
    }
    return value;
}

double CoordinatePrinter::toWgs84Lat(double lat, double lon) const
{
    double value = lat;
    if (_system == CoordinateSystem::SK42) {
        value = sk42ToWgs84Lat(lat, lon, 0);
    }
    return value;
}

double CoordinatePrinter::toWgs84Lon(double lat, double lon) const
{
    double value = lon;
    if (_system == CoordinateSystem::SK42) {
        value = sk42ToWgs84Lon(lat, lon, 0);
    }
    return value;
}

static int decompose(double inputDeg, double* outputMin)
{
    double deg;
    double min = std::modf(inputDeg, &deg);
    *outputMin = std::abs(min * 60);
    return int(deg);
}

QString CoordinatePrinter::print(double lat, double lon, const QString& arg) const
{
    QString latStr = printLat(lat, lon);
    QString lonStr = printLon(lat, lon);
    return arg.arg(latStr).arg(lonStr);
}

QString CoordinatePrinter::printLat(double lat, double lon) const
{
    return printValue(fromWgs84Lat(lat, lon));
}

QString CoordinatePrinter::printLon(double lat, double lon) const
{
    return printValue(fromWgs84Lon(lat, lon));
}

QString CoordinatePrinter::printLatExternal(double lat, double lon) const
{
    return printValueExternal(fromWgs84Lat(lat, lon));
}

QString CoordinatePrinter::printLonExternal(double lat, double lon) const
{
    return printValueExternal(fromWgs84Lon(lat, lon));
}

static QString printDegrees(double value, int prec = 5)
{
    return QString("%1%2").arg(value, 0, 'f', prec).arg(degreeChar);
}

static QString printDegreesMinutes(double value)
{
    double minutesFloat;
    int degrees = decompose(value, &minutesFloat);

    return QString::fromUtf8("%1%2%3'").arg(degrees).arg(degreeChar).arg(minutesFloat, 0, 'f', 3);
}

static QString printDegreesMinutesSeconds(double value)
{
    double minutesFloat;
    double secondsFloat;
    int degrees = decompose(value, &minutesFloat);
    int minutes = decompose(minutesFloat, &secondsFloat);
    return QString::fromUtf8("%1%2%3'%4\"").arg(degrees).arg(degreeChar).arg(minutes).arg(secondsFloat, 0, 'f', 2);
}

LatLonEdit *CoordinatePrinter::latitudeEditor(QWidget* parent) const
{
    return new LatEdit(parent);
}

LatLonEdit *CoordinatePrinter::longitudeEditor(QWidget* parent) const
{
    return new LonEdit(parent);
}

QValidator* CoordinatePrinter::latitudeValidator() const
{
    return new PrinterDegreesValidator(*this, -85., 85.);
}

QValidator* CoordinatePrinter::longitudeValidator() const
{
    return new PrinterDegreesValidator(*this, -180., 180.);
}

QString CoordinatePrinter::latitudeInputMask() const
{
    switch (_format)
    {
        case CoordinateFormat::Degrees:
            return QString("#09.00000000") + degreeChar;
        case CoordinateFormat::DegreesMinutes:
            return QString("#09") + degreeChar + "09.000'";
        case CoordinateFormat::DegreesMinutesSeconds:
            return QString("#09") + degreeChar + "09'09.00\"";
    }
    Q_ASSERT(false);
}

QString CoordinatePrinter::longitudeInputMask() const
{
    switch (_format)
    {
        case CoordinateFormat::Degrees:
            return QString("#009.00000000") + degreeChar;
        case CoordinateFormat::DegreesMinutes:
            return QString("#009") + degreeChar + "09.000'";
        case CoordinateFormat::DegreesMinutesSeconds:
            return QString("#009") + degreeChar + "09'09.00\"";
    }
    Q_ASSERT(false);
}

QString CoordinatePrinter::printValue(double value, int prec) const
{
    switch (_format) {
    case CoordinateFormat::Degrees:
        return printDegrees(value, prec);
    case CoordinateFormat::DegreesMinutes:
        return printDegreesMinutes(value);
    case CoordinateFormat::DegreesMinutesSeconds:
        return printDegreesMinutesSeconds(value);
    }
    Q_ASSERT(false);
}

QString CoordinatePrinter::printValueExternal(double value) const
{
    switch (_format) {
    case CoordinateFormat::Degrees:
        return externalizeString(printDegrees(value));
    case CoordinateFormat::DegreesMinutes:
        return externalizeString(printDegreesMinutes(value));
    case CoordinateFormat::DegreesMinutesSeconds:
        return externalizeString(printDegreesMinutesSeconds(value));
    }
    Q_ASSERT(false);
}

QString CoordinatePrinter::externalizeString(const QString& value) const
{
    static QRegExp notAllowedSymbols("[^0-9\\.\\-\\,]");
    return QString(value).replace(notAllowedSymbols, " ");
}

misc::Option<double> CoordinatePrinter::parseDegrees(const QString& line) const
{
    bool isOk;
    double res = line.left(line.size() - 1).toDouble(&isOk);
    if (isOk) {
        return res;
    }
    return misc::None;
}

misc::Option<double> CoordinatePrinter::parseDegreesMinutes(const QString& line) const
{
    bool isOk;
    QStringList list = line.split(degreeChar);
    if (list.size() != 2) {
        return misc::None;
    }
    int deg = list[0].toInt(&isOk);
    if (!isOk) {
        return misc::None;
    }
    const QString& minStr = list[1];
    if (minStr.size() < 2 || minStr[minStr.size() - 1] != '\'') {
        return misc::None;
    }
    double min = minStr.left(minStr.size() - 1).toDouble(&isOk);
    if (!isOk || min < 0) {
        return misc::None;
    }
    return deg + min / 60.;
}

misc::Option<double> CoordinatePrinter::parseDegreesMinutesSeconds(const QString& line) const
{
    bool isOk;
    QStringList list = line.split(degreeChar);
    if (list.size() != 2) {
        return misc::None;
    }
    int deg = list[0].toInt(&isOk);
    if (!isOk) {
        return misc::None;
    }
    const QString& minSecStr = list[1];
    QStringList minSecList = minSecStr.split('\'');
    if (minSecList.size() != 2) {
        return misc::None;
    }
    const QString& minStr = minSecList[0];
    int min = minStr.toUInt(&isOk);
    if (!isOk || min < 0) {
        return misc::None;
    }
    const QString& secStr = minSecList[1];
    if (secStr.size() < 2 || secStr[secStr.size() - 1] != '"') {
        return misc::None;
    }
    double sec = secStr.left(secStr.size() - 1).toDouble(&isOk);
    if (!isOk) {
        return misc::None;
    }
    int sign(deg >= 0 ? 1 : -1);
    return deg + sign * min / 60. + sign * sec / 3600.;
}

misc::Option<double> CoordinatePrinter::parse(const QString& line) const
{
    switch (_format) {
    case CoordinateFormat::Degrees:
        return parseDegrees(line);
    case CoordinateFormat::DegreesMinutes:
        return parseDegreesMinutes(line);
    case CoordinateFormat::DegreesMinutesSeconds:
        return parseDegreesMinutesSeconds(line);
    }
    Q_ASSERT(false);
}

void CoordinatePrinter::setFormat(CoordinateFormat format)
{
    _format = format;
}

void CoordinatePrinter::setTargetSystem(CoordinateSystem system)
{
    _system = system;
}
}
}
}
