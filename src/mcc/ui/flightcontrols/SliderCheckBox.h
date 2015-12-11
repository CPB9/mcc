/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#pragma once

#include <QCheckBox>
#include <QPainter>
#include <QVariant>

#include "mcc/misc/Net.h"

namespace mcc {
    namespace ui {
        namespace ide {

            class SliderCheckBox : public QCheckBox
            {
                Q_OBJECT

            public:
                explicit SliderCheckBox(QWidget* parent = 0);

                SliderCheckBox(const mcc::misc::NetChannel& netAddress, const QString& exchangeService, int protocolId = 0, QWidget* parent = 0);

                void setNetAddress(const mcc::misc::NetChannel& netAddress);
                void setProtocolId(int protocolId);
                void setExchangeService(const QString& exchangeService);

                virtual void mousePressEvent(QMouseEvent* e) override;
                virtual void mouseReleaseEvent(QMouseEvent *) override;

                virtual void paintEvent(QPaintEvent *e) override;

                virtual QSize sizeHint() const override;

                const mcc::misc::NetChannel& netAddress() const;
                int protocolId() const;
                QString exchangeService() const;
            signals:
                void sliderStateChanged(bool state);



            private:
                QString _exchangeService;
                mcc::misc::NetChannel _netAddress;
                int _protocolId;
            };

        }
    }
}

