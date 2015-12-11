/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "mcc/ui/flightcontrols/PrimaryFlightDisplay.h"

#include "mcc/ui/core/Settings.h"

#include <QPainter>
#include <QDebug>

namespace mcc {
    namespace ui {
        namespace flightcontrols {

            inline double min4(double a, double b, double c, double d) {
                if (b < a) a = b;
                if (c < a) a = c;
                if (d < a) a = d;
                return a;
            }

            inline double max4(double a, double b, double c, double d) {
                if (b > a) a = b;
                if (c > a) a = c;
                if (d > a) a = d;
                return a;
            }

            PrimaryFlightDisplay::PrimaryFlightDisplay(mcc::ui::core::DeviceManager* manager, QWidget *parent)
                : QWidget(parent)
                , _aircraftModel(nullptr)
                , _pitch(0.0)
                , _roll(0.0)
                , _speed(0.0)
                , _altitude(0.0)
                , _heading(0.0)
                , _targetSpeed(0.0)
                , _targetAltitude(0.0)
                , _targetHeading(0.0)
                , _invertedIndication(true)
            {
                using mcc::ui::core::Settings;

                setMinimumSize(200, 200);

                if (manager != nullptr)
                    connect(manager, &mcc::ui::core::DeviceManager::deviceRemoved, this, [this](mcc::ui::core::FlyingDevice* device) {if (_aircraftModel == device) _aircraftModel = nullptr; });

                connect(Settings::instance(), &Settings::invertedPfdChanged, this, &PrimaryFlightDisplay::setInvertedIndication);

                _invertedIndication = Settings::instance()->invertedPfd();
            }

            PrimaryFlightDisplay::~PrimaryFlightDisplay()
            {

            }

            void PrimaryFlightDisplay::setRoll(double roll)
            {
                _roll = roll;
                update();
            }

            void PrimaryFlightDisplay::setPitch(double pitch)
            {
                _pitch = pitch;
                update();
            }

            void PrimaryFlightDisplay::init()
            {
            }

            void PrimaryFlightDisplay::setAirSpeed(double airSpeed)
            {
                _speed = airSpeed;
                update();
            }

            void PrimaryFlightDisplay::setAltitude(double altitude)
            {
                _altitude = altitude;
                update();
            }

            void PrimaryFlightDisplay::setHeading(double heading)
            {
                _heading = heading;

                while (_heading < 0)
                    _heading += 360;

                while (_heading >= 360)
                    _heading -= 360;

                update();
            }

            void PrimaryFlightDisplay::setTargetHeading(double targetHeading)
            {
                _targetHeading = targetHeading;
                update();
            }

            void PrimaryFlightDisplay::setModel(mcc::ui::core::FlyingDevice* model)
            {
                if (model == nullptr)
                    return;

                if (_aircraftModel != nullptr)
                    disconnect(_aircraftModel, 0, this, 0);

                _aircraftModel = model;

                if (_aircraftModel->isActive())
                {

                    setPosition(_aircraftModel->position());
                    setOrientation(_aircraftModel->orientation());
                    setAirSpeed(_aircraftModel->speed());
                    setTargetHeading(_aircraftModel->targetHeading());

                    mcc::ui::core::Route* route = _aircraftModel->activeRoute();
                    if (route && route->waypointsCount() > 0)
                        setNextWaypoint(_aircraftModel->nextWaypoint());
                }

                connect(_aircraftModel, &mcc::ui::core::FlyingDevice::positionChanged,      this, &PrimaryFlightDisplay::setPosition);
                connect(_aircraftModel, &mcc::ui::core::FlyingDevice::orientationChanged,   this, &PrimaryFlightDisplay::setOrientation);
                connect(_aircraftModel, &mcc::ui::core::FlyingDevice::speedChanged,         this, &PrimaryFlightDisplay::setAirSpeed);
                connect(_aircraftModel, &mcc::ui::core::FlyingDevice::targetHeadingChanged, this, &PrimaryFlightDisplay::setTargetHeading);
                connect(_aircraftModel, &mcc::ui::core::FlyingDevice::nextWaypointChanged,  this, &PrimaryFlightDisplay::setNextWaypoint);
            }

            void PrimaryFlightDisplay::setPosition(const mcc::ui::core::GeoPosition& position)
            {
                setAltitude(position.altitude);
            }

            void PrimaryFlightDisplay::setOrientation(const mcc::ui::core::GeoOrientation& orientation)
            {
                setRoll(orientation.roll);
                setPitch(orientation.pitch);
                setHeading(orientation.heading);
            }

            void PrimaryFlightDisplay::setNextWaypoint(int idx)
            {
                if (!_aircraftModel->activeRoute())
                    return;

                if (_aircraftModel->activeRoute()->waypointsCount() == 0)
                    return;

                mcc::ui::core::Waypoint waypoint = _aircraftModel->activeRoute()->waypointAt(idx);
                setReferenceSpeed(waypoint.speed);
                setReferenceAltitude(waypoint.position.altitude);
            }

            void PrimaryFlightDisplay::setReferenceSpeed(double speed)
            {
                _targetSpeed = speed;
                update();
            }

            void PrimaryFlightDisplay::setReferenceAltitude(double altitude)
            {
                _targetAltitude = altitude;
                update();
            }

            void PrimaryFlightDisplay::paintEvent(QPaintEvent*)
            {
                QPainter painter(this);
                painter.setRenderHint(QPainter::Antialiasing, true);
                painter.setRenderHint(QPainter::TextAntialiasing, true);
                painter.setRenderHint(QPainter::HighQualityAntialiasing, true);

                drawSkyGround(painter);
                drawPitchScale(painter);
                drawRollScale(painter);
                drawCompass(painter);
                drawAirFrame(painter);
                drawIndicators(painter);
                drawUserData(painter);
            }

            void PrimaryFlightDisplay::drawSkyGround(QPainter& painter)
            {
                painter.resetTransform();

                QRectF skyGroundArea(0, 0, width(), height());

                painter.translate(skyGroundArea.center());
                if (!_invertedIndication)
                    painter.rotate(-_roll);

                painter.translate(0, pitchTranslate(_pitch));

                QTransform invertTransform = painter.transform().inverted();
                QRectF invertRect = invertTransform.mapRect(skyGroundArea);

                double minX = min4(invertRect.topLeft().x(), invertRect.topRight().x(), invertRect.bottomLeft().x(), invertRect.bottomRight().x());
                double maxX = max4(invertRect.topLeft().x(), invertRect.topRight().x(), invertRect.bottomLeft().x(), invertRect.bottomRight().x());
                double minY = min4(invertRect.topLeft().y(), invertRect.topRight().y(), invertRect.bottomLeft().y(), invertRect.bottomRight().y());
                double maxY = max4(invertRect.topLeft().y(), invertRect.topRight().y(), invertRect.bottomLeft().y(), invertRect.bottomRight().y());

                double end = pitchTranslate(60);

                QLinearGradient gradient(0, -end, 0, 0);
                gradient.setColorAt(0, QColor::fromRgb(0, 31, 87));
                gradient.setColorAt(1, QColor::fromRgb(130, 195, 253));

                QBrush skyBrush(gradient);
                QPainterPath skyPath;
                skyPath.moveTo(minX, 0);
                skyPath.lineTo(minX, minY);
                skyPath.lineTo(maxX, minY);
                skyPath.lineTo(maxX, 0);
                skyPath.lineTo(minX, 0);

                painter.fillPath(skyPath, skyBrush);

                gradient = QLinearGradient(0, end, 0, 0);
                gradient.setColorAt(0, QColor::fromRgb(105, 81, 19));
                gradient.setColorAt(1, QColor::fromRgb(254, 206, 83));
                QBrush groundBrush(gradient);

                QPainterPath groundPath;
                groundPath.moveTo(minX, 0);
                groundPath.lineTo(minX, maxY);
                groundPath.lineTo(maxX, maxY);
                groundPath.lineTo(maxX, 0);
                groundPath.lineTo(minX, 0);

                painter.fillPath(groundPath, groundBrush);

                QPen pen;
                pen.setColor(Qt::white);
                pen.setWidth(2);

                painter.setPen(pen);
                painter.drawLine(minX, 0, maxX, 0);
            }

            double PrimaryFlightDisplay::pitchTranslate(double pitch) const
            {
                return (double)scaleBase(rect()) / BASE_WIDTH * BASE_PITCH_SCALE * pitch;
            }

            void PrimaryFlightDisplay::drawPitchScale(QPainter& painter)
            {
                painter.resetTransform();

                QRectF skyGroundArea(0, 0, width(), height());

                painter.translate(skyGroundArea.center());
                if (!_invertedIndication)
                    painter.rotate(-_roll);

                QTransform savedTransform = painter.transform();

                int startPitch = qRound((double)(_pitch / BASE_PITCH_SCALE_INTERVAL))*BASE_PITCH_SCALE_INTERVAL;
                startPitch -= 15;

                int maxPitch = startPitch + 30;

                QPen pen(Qt::white);
                painter.setPen(pen);

                double normalWidth = 0.13 * scaleBase(skyGroundArea);

                double width = normalWidth;

                for (int currentPitch = startPitch; currentPitch <= maxPitch; currentPitch += BASE_PITCH_SCALE_INTERVAL)
                {
                    painter.translate(0, pitchTranslate(_pitch - currentPitch));

                    bool isMajor = ((currentPitch % (BASE_PITCH_SCALE_INTERVAL*2)) == 0);
                    if (isMajor)
                    {
                        pen.setWidthF(1.0);
                        pen.setCapStyle(Qt::RoundCap);

                        painter.setPen(pen);

                        painter.drawLine(-width, 0, -width/2, 0);
                        painter.drawLine(width / 2, 0, width, 0);

                        int displayPitch = currentPitch;
                        if (displayPitch > 90)
                            displayPitch = 180 - currentPitch;
                        else if (displayPitch < -90)
                            displayPitch = -180 - currentPitch;

                        double fontSize = scaleBase(skyGroundArea) * _normalFontSize;

                        drawText(painter, QString::number(displayPitch), fontSize, -width - fontSize * 2, 0, Qt::AlignLeft);
                        drawText(painter, QString::number(displayPitch), fontSize, width + fontSize * 2, 0, Qt::AlignLeft);
                    }
                    else
                    {
                        pen.setWidthF(0.01 * scaleBase(skyGroundArea));
                        painter.setPen(pen);
                        painter.drawPoint(0, 0);
                    }

                    painter.setTransform(savedTransform);
                }
            }

            void PrimaryFlightDisplay::drawAirFrame(QPainter& painter)
            {
                painter.resetTransform();

                QRectF skyGroundArea(0, 0, width(), height());

                painter.translate(skyGroundArea.center());
                if (_invertedIndication)
                    painter.rotate(_roll);

                double width = scaleBase(skyGroundArea);

                QPen pen(Qt::red);
                pen.setWidth(2);
                painter.setPen(pen);

                QPainterPath airplanePath(QPointF(-0.15 * width, 0));
                airplanePath.lineTo(-0.1 * width, 0);
                airplanePath.lineTo(-0.05 * width, 0.05 * width);
                airplanePath.lineTo(0, 0);
                airplanePath.lineTo(0.05 * width, 0.05 * width);
                airplanePath.lineTo(0.1 * width, 0);
                airplanePath.lineTo(0.15 * width, 0);

                painter.drawPath(airplanePath);
            }

            void PrimaryFlightDisplay::drawRollScale(QPainter& painter)
            {
                painter.resetTransform();

                QRectF skyGroundArea(0, 0, width(), height());

                painter.translate(skyGroundArea.center());
                double size = 0.8 * scaleBase(skyGroundArea);

                QRectF arcArea(-size / 2, -size / 2, size, size);

                int ROLL_SCALE_RANGE = 60;
                int RESOLUTION = 10;

                QPen pen(Qt::white);
                pen.setWidth(1);

                painter.setPen(pen);

                QTransform savedTransform = painter.transform();
                for (int angle = -ROLL_SCALE_RANGE; angle <= ROLL_SCALE_RANGE; angle += RESOLUTION)
                {
                    //painter.translate(0, pitchTranslate(_pitch - currentPitch));
                    painter.rotate(angle);

                    double length = 0.02 * scaleBase(skyGroundArea);
                    double textPosY = length * 2;

                    bool isMajor = ((angle % (RESOLUTION * 2)) == 0);
                    if (isMajor)
                    {
                        length *= 2;
                    }

                    painter.drawLine(0, size / 2, 0, size / 2 + length);

                    double fontSize = scaleBase(skyGroundArea) * _normalFontSize;

                    double textPosX = fontSize / 4;
                    if (angle != 0)
                        textPosX = fontSize / 2;

                    if (_invertedIndication)
                        drawText(painter, QString::number(angle), fontSize, textPosX, size / 2 + textPosY + fontSize / 2, Qt::AlignLeft);
                    else
                        drawText(painter, QString::number(-angle), fontSize, textPosX, size / 2 + textPosY + fontSize / 2, Qt::AlignLeft);


                    painter.setTransform(savedTransform);
                }


                pen = QPen(Qt::white);
                pen.setWidth(2);
                painter.setPen(pen);

                painter.drawArc(arcArea, 210 * 16, ROLL_SCALE_RANGE * 2 * 16);

                if (_invertedIndication)
                    painter.rotate(_roll);
                else
                    painter.rotate(-_roll);

                QPainterPath rollMarkerPath;
                double x = scaleBase(skyGroundArea) * 0.05;

                rollMarkerPath.lineTo(-x / 2, 0);
                rollMarkerPath.lineTo(x / 2, 0);
                rollMarkerPath.lineTo(0, x);
                rollMarkerPath.lineTo(-x/2, 0);

                painter.translate(QPointF(0, scaleBase(skyGroundArea) * 0.35));

                painter.fillPath(rollMarkerPath, Qt::white);

            }

            void PrimaryFlightDisplay::drawIndicators(QPainter& painter)
            {
                painter.resetTransform();

                QRectF skyGroundArea(0, 0, width(), height());

                painter.translate(skyGroundArea.center());

                double scaleFactor = scaleBase(skyGroundArea);
                double posX = 0.35 * scaleFactor;

                QPen pen(Qt::white);
                painter.setPen(pen);

                double fontSize = scaleBase(skyGroundArea) * _normalFontSize;

                double backgroundWidth = 0.15 * scaleFactor;
                double backgroundHeight = 0.4 * scaleFactor;

                double backgroundPos = 0.47 * scaleFactor;

                painter.fillRect(-backgroundPos                 , -backgroundHeight / 2, backgroundWidth, backgroundHeight, _indicatorBackgroundColor);
                painter.fillRect(backgroundPos - backgroundWidth, -backgroundHeight / 2, backgroundWidth, backgroundHeight, _indicatorBackgroundColor);

                drawText(painter, QString("%1").arg((int)_speed), fontSize, -posX, 0, Qt::AlignLeft);
                drawText(painter, QString("%1").arg((int)_altitude), fontSize, posX + backgroundWidth / 2, 0, Qt::AlignLeft);

                double backgroundHeight2 = backgroundHeight / 4;

                pen.setColor(Qt::yellow);

                painter.setPen(pen);
                painter.fillRect(-backgroundPos, -backgroundHeight / 2 - backgroundHeight2 - 1, backgroundWidth, backgroundHeight2, _indicatorBackgroundColor);
                painter.fillRect(backgroundPos - backgroundWidth, -backgroundHeight / 2 - backgroundHeight2 - 1, backgroundWidth, backgroundHeight2, _indicatorBackgroundColor);
//
                drawText(painter, QString("TS:%1").arg((int)_targetSpeed), fontSize * 0.8, -posX, -backgroundHeight / 2 - backgroundHeight2 + fontSize, Qt::AlignLeft);
                drawText(painter, QString("AL:%1").arg((int)_targetAltitude), fontSize * 0.8, posX + backgroundWidth/2, -backgroundHeight / 2 - backgroundHeight2 + fontSize, Qt::AlignLeft);

            }

            void PrimaryFlightDisplay::drawCompass(QPainter& painter)
            {
                painter.resetTransform();

                QRectF area(0, 0, width(), height());

                painter.translate(area.center());

                double width = scaleBase(area);

                QPen pen(Qt::yellow);
                pen.setWidth(2);

                painter.setPen(pen);

                double x = width * 0.05;

                QPainterPath rollMarkerPath;
                rollMarkerPath.lineTo(x / 2, x);
                rollMarkerPath.lineTo(-x / 2, x);
                rollMarkerPath.lineTo(0, 0);

                painter.translate(0, -width * 0.37);

                painter.fillPath(rollMarkerPath, Qt::yellow);

                const int COMPASS_RESOLUTION = 5;
                const int COMPASS_SCALE = 60;

                int startHeading = qRound((double)(_heading / COMPASS_RESOLUTION))*COMPASS_RESOLUTION;

                startHeading -= COMPASS_SCALE / 2;

                int stopHeading = startHeading + COMPASS_SCALE;

                painter.translate(0, -width * 0.02);

                pen.setColor(Qt::white);
                painter.setPen(pen);


                QTransform savedTransform = painter.transform();

                double height = 0.02;
                for (int heading = startHeading; heading <= stopHeading; heading += COMPASS_RESOLUTION)
                {
                    painter.translate(-(_heading - heading) * width * 0.01, 0);

                    bool isMajor = ((heading % (COMPASS_RESOLUTION * 2)) == 0);
                    double h;

                    if (isMajor)
                    {
                        h = height;
                    }
                    else
                        h = height / 2;

                    QPointF start(0, -h / 2 * width);
                    QPointF stop(0, h / 2 * width);

                    painter.drawLine(start, stop);

                    bool isMajorMajor = ((heading % (COMPASS_RESOLUTION * 2)) == 0);
                    if (isMajorMajor)
                    {
                        int displayHeading = heading;

                        if (displayHeading > 359)
                            displayHeading = heading - 360;
                        else if (displayHeading < 0)
                            displayHeading = heading + 360;

                        QString displayString;
                        switch (displayHeading)
                        {
                        case 0:
                            displayString = "N";
                            break;
                        case 90:
                            displayString = "E";
                            break;
                        case 180:
                            displayString = "S";
                            break;
                        case 270:
                            displayString = "W";
                            break;
                        default:
                            displayString = QString("%1").arg((int)displayHeading);
                        }

                        drawText(painter, displayString, _normalFontSize * width, 0, -4* h * width, Qt::AlignCenter);
                    }

                    painter.setTransform(savedTransform);
                }

                double valueBackgroundWidth = height * 3 * width;

                pen.setWidth(3);
                painter.setPen(pen);
                painter.fillRect(-valueBackgroundWidth, -valueBackgroundWidth * 1.5, valueBackgroundWidth * 2, valueBackgroundWidth, _indicatorBackgroundColor);

                QString headingText = QString("%1").arg(_heading, 3, 'f', 0, QChar('0'));

                drawText(painter, headingText, width * _normalFontSize, 0, -valueBackgroundWidth - width * 0.02, Qt::AlignCenter);

                pen.setWidth(3);
                pen.setColor(Qt::yellow);

                painter.setPen(pen);
                painter.fillRect(-width * 0.47, -valueBackgroundWidth * 1.5, valueBackgroundWidth * 2.5, valueBackgroundWidth, _indicatorBackgroundColor);

                headingText = QString("TH:%1").arg(_targetHeading, 3, 'f', 0, QChar('0'));

                drawText(painter, headingText, width * _normalFontSize * 0.8, -width * 0.42 + valueBackgroundWidth/2, -valueBackgroundWidth - width * 0.02, Qt::AlignCenter);
            }

            void PrimaryFlightDisplay::drawText(QPainter& painter, const QString& text, double fontSize, double x, double y, Qt::Alignment alignment)
            {
                QFont font;
                font.setPixelSize(fontSize);
                painter.setFont(font);

                QFontMetrics metrics(font);
                QRect bounds = metrics.boundingRect(text);
                int flags = alignment;

                QRectF rect;
                switch (alignment)
                {
                case Qt::AlignLeft:
                    rect = QRectF(x - bounds.width(), y - bounds.height() / 2, bounds.width() + 1, bounds.height());
                    break;
                case Qt::AlignCenter:
                    rect = QRectF(x - bounds.width() / 2, y, bounds.width(), bounds.height());
                    break;
                case Qt::AlignRight:
                    rect = QRectF(x, y - bounds.height() / 2, bounds.width(), bounds.height());
                    break;
                default:
                    Q_ASSERT(false);
                }

                painter.drawText(rect, flags, text);
            }

            double PrimaryFlightDisplay::scaleBase(const QRectF& area) const
            {
                return qMin(area.width(), area.height());
            }

            void PrimaryFlightDisplay::drawUserData(QPainter& painter)
            {
                Q_UNUSED(painter);
            }

            bool PrimaryFlightDisplay::invertedIndication() const
            {
                return _invertedIndication;
            }

            void PrimaryFlightDisplay::setInvertedIndication(bool indication)
            {
                _invertedIndication = indication;
            }

        }
    }
}
