/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#pragma once

#include <QWidget>

#include "mcc/ui/core/Structs.h"
#include "mcc/ui/core/FlyingDevice.h"
#include "mcc/ui/core/DeviceManager.h"

namespace mcc {
    namespace ui {
        namespace flightcontrols {

            class PrimaryFlightDisplay : public QWidget
            {
                Q_OBJECT

            public:
                PrimaryFlightDisplay(mcc::ui::core::DeviceManager* manager, QWidget* parent = 0);
                ~PrimaryFlightDisplay();

                void init();

                bool invertedIndication() const;
             public slots:
                void setRoll(double roll);
                void setPitch(double pitch);

                void setPosition(const mcc::ui::core::GeoPosition& position);
                void setOrientation(const mcc::ui::core::GeoOrientation& orientation);

                void setAirSpeed(double airSpeed);
                void setReferenceSpeed(double speed);

                void setAltitude(double altitude);
                void setReferenceAltitude(double altitude);

                void setHeading(double heading);
                void setTargetHeading(double targetHeading);
                void setNextWaypoint( int idx);

                void setInvertedIndication(bool indication);

                void setModel(mcc::ui::core::FlyingDevice* model);

            protected:
                virtual void paintEvent(QPaintEvent *) override;

                void drawSkyGround(QPainter& painter);
                void drawPitchScale(QPainter& painter);
                void drawAirFrame(QPainter& painter);
                void drawRollScale(QPainter& painter);
                void drawIndicators(QPainter& painter);
                void drawCompass(QPainter& painter);
                void drawUserData(QPainter& painter);

                void drawText(QPainter& painter, const QString& text, double fontSize, double x, double y, Qt::Alignment alignment);

                double pitchTranslate(double pitch) const;

                double scaleBase(const QRectF& area) const;

            private:
                mcc::ui::core::FlyingDevice* _aircraftModel;

                const double BASE_WIDTH = 300;
                const double BASE_PITCH_SCALE = 5;
                const int BASE_PITCH_SCALE_INTERVAL = 5;
                const QColor _indicatorBackgroundColor = QColor(0, 0, 0, 150);
                const double _normalFontSize = 0.045;


                double _pitch;
                double _roll;
                double _speed;
                double _altitude;
                double _heading;

                double _targetSpeed;
                double _targetAltitude;
                double _targetHeading;

                bool   _invertedIndication;
            };

        }
    }
}
