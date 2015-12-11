/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "TmParameterWidget.h"

#include <QGridLayout>

#include <QIcon>

namespace mcc {
    namespace ui {
        namespace flightcontrols {

            TmParameterWidget::TmParameterWidget(const QIcon& upIcon, const QIcon& downIcon, const QIcon& equalIcon, QWidget* parent /* = 0 */)
                : QFrame(parent)
            {
                const int pixmapSize = 16;
                _upPixmap = upIcon.pixmap(pixmapSize, pixmapSize);
                _downPixmap = downIcon.pixmap(pixmapSize, pixmapSize);
                _eqPixmap = equalIcon.pixmap(pixmapSize, pixmapSize);

                setFrameShape(QFrame::Box);
                setFrameShadow(QFrame::Plain);

                _name = new QLabel("--");
                _value = new QLabel("--");
                _trend = new QLabel();
                _time = new QLabel("--:--");

                setStyleSheet("color: rgb(159, 159, 159);");

                _name->setStyleSheet("color: rgb(131, 131, 131);");
                _value->setStyleSheet("QLabel{\n	color: rgb(0, 0, 0);\n	font: bold 8pt \"MS Shell Dlg 2\";\n}");

                auto layout = new QGridLayout();
                layout->addWidget(_name, 0, 0, 1, 3, Qt::AlignCenter);
                layout->addWidget(_trend, 1, 0, Qt::AlignLeft);
                layout->addWidget(_value, 1, 1);
                layout->addWidget(_time, 1, 2, Qt::AlignRight);

                setLayout(layout);
            }

            void TmParameterWidget::setName(const QString& name)
            {
                _name->setText(name);
            }



            void TmParameterWidget::setValue(const QString& value)
            {
                _value->setText(value);
            }

            void TmParameterWidget::setTime(const QString& time)
            {
                _time->setText(time);
            }

            void TmParameterWidget::setUp()
            {
                _trend->setPixmap(_upPixmap);
            }

            void TmParameterWidget::setDown()
            {
                _trend->setPixmap(_downPixmap);
            }

            void TmParameterWidget::setEqual()
            {
                _trend->setPixmap(_eqPixmap);
            }

        }
    }
}