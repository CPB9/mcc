/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#pragma once

#include <QFrame>
#include <QLabel>
#include <QDateTime>

#include "mcc/misc/TmParam.h"

namespace mcc {
    namespace ui {
        namespace flightcontrols {

            class TmParameterWidget : public QFrame
            {
                Q_OBJECT

            public:
                TmParameterWidget(const QIcon& upIcon, const QIcon& downIcon, const QIcon& equalIcon, QWidget* parent = 0);

            public slots:
                void setName(const QString& name);
                void setValue(const QString& value);
                void setTime(const QString& time);

                void setUp();
                void setDown();
                void setEqual();

            private:
                QLabel* _name;
                QLabel* _value;
                QLabel* _trend;
                QLabel* _time;

                QPixmap _upPixmap;
                QPixmap _downPixmap;
                QPixmap _eqPixmap;
            };
        }
    }
}