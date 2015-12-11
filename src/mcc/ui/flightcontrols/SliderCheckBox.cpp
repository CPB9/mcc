/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "SliderCheckBox.h"

#include <QMouseEvent>

namespace mcc {
    namespace ui {
        namespace ide {


            SliderCheckBox::SliderCheckBox(QWidget* parent /*= 0*/) : QCheckBox(parent)
            {
                setTristate(true);

                setStyleSheet("background-color: rgb(0, 0, 0);\n" \
                    "color: rgb(255, 255, 255);\n");
            }


            SliderCheckBox::SliderCheckBox(const mcc::misc::NetChannel& netAddress, const QString& exchangeService, int protocolId, QWidget* parent /*= 0*/)
                : SliderCheckBox(parent)
            {
                setNetAddress(netAddress);
                setProtocolId(protocolId);
                setExchangeService(exchangeService);
            }

            void SliderCheckBox::setNetAddress(const mcc::misc::NetChannel& netAddress)
            {
                _netAddress = netAddress;
            }


            void SliderCheckBox::setProtocolId(int protocolId)
            {
                _protocolId = protocolId;
            }

            void SliderCheckBox::mousePressEvent(QMouseEvent* e)
            {
                if (checkState() == Qt::Checked)
                    emit sliderStateChanged(false);
                else if (checkState() == Qt::Unchecked)
                    emit sliderStateChanged(true);
                e->accept();
            }

            void SliderCheckBox::paintEvent(QPaintEvent *e)
            {
                Q_UNUSED(e);

                resize(sizeHint());

                QPainter painter(this);
                painter.setRenderHint(QPainter::Antialiasing);

                QPen pen;
                pen.setColor(Qt::black);
                painter.setPen(pen);

                QBrush uncheckedBackgroundBrush(Qt::black);
                QBrush partiallyCheckedBackgroundBrush(Qt::gray);
                QBrush checkedBackgroundBrush(Qt::darkGreen);

                QBrush markerBrush(QColor(240, 240, 240));

                int ellipseSize = height() - 1;

                switch (checkState())
                {
                case Qt::Checked:
                {
                    painter.setBrush(checkedBackgroundBrush);
                    painter.drawRoundedRect(0, 0, width() - 2, height() - 2, height() / 2, height() / 2);

                    painter.setBrush(markerBrush);
                    painter.drawEllipse(width() - height(), 0, ellipseSize, ellipseSize);
                    break;
                }
                case Qt::PartiallyChecked:
                {
                    painter.setBrush(partiallyCheckedBackgroundBrush);
                    painter.drawRoundedRect(0, 0, width() - 2, height() - 2, height() / 2, height() / 2);

                    painter.setBrush(markerBrush);
                    painter.drawEllipse(width() / 2 - height() / 2, 0, ellipseSize, ellipseSize);
                    break;
                }
                case Qt::Unchecked:
                {
                    painter.setBrush(uncheckedBackgroundBrush);
                    painter.drawRoundedRect(0, 0, width() - 2, height() - 2, height() / 2, height() / 2);

                    painter.setBrush(markerBrush);
                    painter.drawEllipse(0, 0, ellipseSize, ellipseSize);
                    break;
                }
                }
            }

            QSize SliderCheckBox::sizeHint() const
            {
                return QSize(40, 15);
            }

            const mcc::misc::NetChannel& SliderCheckBox::netAddress() const
            {
                return _netAddress;
            }

            int SliderCheckBox::protocolId() const
            {
                return _protocolId;
            }

            void SliderCheckBox::mouseReleaseEvent(QMouseEvent* e)
            {
                e->accept();
            }

            void SliderCheckBox::setExchangeService(const QString& exchangeService)
            {
                _exchangeService = exchangeService;
            }

            QString SliderCheckBox::exchangeService() const
            {
                return _exchangeService;
            }

        }
    }
}

