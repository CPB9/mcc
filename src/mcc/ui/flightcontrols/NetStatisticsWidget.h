/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#pragma once

#include <memory>
#include <array>

#include <qwt_plot.h>
#include <qwt_plot_curve.h>
#include <qwt_plot_canvas.h>
#include <qwt_plot_layout.h>
#include <qwt_plot_textlabel.h>
#include <qwt_scale_draw.h>

#include "mcc/misc/NetStatistics.h"

namespace mcc {
    namespace ui {
        namespace flightcontrols {

            class ScaleDraw : public QwtScaleDraw
            {
            public:
                virtual void drawTick(QPainter *, double val, double len) const override
                {
                }

                virtual void drawBackbone(QPainter *) const override
                {
                }

                virtual void drawLabel(QPainter *, double val) const override
                {
                }

                virtual void draw(QPainter *, const QPalette &) const override
                {
                }

                virtual double extent(const QFont &) const override
                {
                    return 0.0;
                }

                virtual QwtText label(double) const override
                {
                    return QwtText();
                }
            };


            class NetStatisticsWidget : public QwtPlot
            {
                static const size_t REFRESH_RATE = 10;           // 1/10 s chart update
                static const size_t WATCH_DOG = 2;               // 2 s watchdog
                static const size_t HISTORY = 10 * REFRESH_RATE; // curves history

                class Curve
                {
                public:
                    enum Kind
                    {
                        SentBytes,
                        SentPackets,
                        ReceivedBytes,
                        ReceivedPackets,
                        ReceivedBadBytes,
                        ReceivedBadPackets,
                        Count
                    };

                    Curve()
                        : _lastValue(0.0)
                        ,_speed(0.0)
                    {
                        _curve = new QwtPlotCurve();

                        for (size_t i = 0; i < HISTORY; ++i)
                        {
                            _data[i] = 0;
                        }
                    }

                    ~Curve()
                    {
                        delete _curve;
                    }

                    void setStyle(const QPen& pen, const QBrush& brush)
                    {
                        _curve->setPen(pen);
                        _curve->setBrush(brush);
                    }

                    void update(const QDateTime& time, double value)
                    {
                        const quint64 watchdog = WATCH_DOG * 1000;

                        if (!_lastUpdate.isValid())
                        {
                            _lastUpdate = time;
                            _lastValue = value;
                            return;
                        }

                        quint64 valuesDelta = time.toMSecsSinceEpoch() - _lastUpdate.toMSecsSinceEpoch();
                        quint64 nowDelta = QDateTime::currentMSecsSinceEpoch() - time.toMSecsSinceEpoch();

                        if (nowDelta > watchdog)
                        {
                            _data[0] = 0.0;
                            _lastUpdate = time;
                            _lastValue = value;
                            _speed = 0.0;
                            return;
                        }

                        if (valuesDelta > 0.0)
                        {
                            _speed = 1000.0 * (value - _lastValue) / valuesDelta;
                        }

                        _data[0] = _speed;
                        _lastUpdate = time;
                        _lastValue = value;
                    }

                    void attach(QwtPlot* plot)
                    {
                        _curve->attach(plot);
                    }

                    void setTimeData(const double* timeValue)
                    {
                        _curve->setRawSamples(timeValue, _data.data(), (int)_data.size());
                    }

                    void shiftLeft()
                    {
                        for (int i = HISTORY - 1; i > 0; i--)
                            _data[i] = _data[i - 1];
                    }

                    void setVisible(bool visible)
                    {
                        _curve->setVisible(visible);
                    }

                    double speed() const
                    {
                        return _speed;
                    }

                    unsigned int maxElement() const
                    {
                        return qRound(*std::max_element(_data.begin(), _data.end()));
                    }

                private:
                    std::array<double, HISTORY> _data;
                    QwtPlotCurve*               _curve;
                    QDateTime                   _lastUpdate;
                    double                      _lastValue;
                    double                      _speed;
                };

                typedef std::unique_ptr<Curve> CurvePtr;

            public:
                NetStatisticsWidget(QWidget* parent = 0)
                    : QwtPlot(parent)
                    , _maxValue(0.0)
                {
                    setAutoReplot(false);

                    auto canvas = new QwtPlotCanvas();
                    canvas->setFrameStyle(QFrame::Box | QFrame::Plain);
                    canvas->setLineWidth(1);
                    canvas->setContentsMargins(0, 0, 0, 0);
                    setCanvas(canvas);

                    plotLayout()->setAlignCanvasToScales(true);

                    setAxisScale(QwtPlot::xBottom, 0, HISTORY);
                    setAxisScaleDraw(QwtPlot::yLeft, new ScaleDraw());
                    setAxisScaleDraw(QwtPlot::xBottom, new ScaleDraw());

                    enableAxis(QwtPlot::yLeft, false);
                    enableAxis(QwtPlot::xBottom, false);

                    setAxisAutoScale(QwtPlot::xBottom, false);
                    setAxisAutoScale(QwtPlot::yLeft, false);

                    for (int i = 0; i < Curve::Count; ++i)
                    {
                        _curves[i] = CurvePtr(new Curve());
                    }

                    _curves[Curve::SentBytes]->attach(this);
                    _curves[Curve::SentBytes]->setStyle(QPen(QColor("#e68a5c"), 1.0, Qt::DashLine), QColor(255, 255, 255, 0));

                    _curves[Curve::ReceivedBytes]->attach(this);
                    _curves[Curve::ReceivedBytes]->setStyle(QPen(QColor("#a74f01"), 1.0), QColor(255, 255, 255, 0));

                    _curves[Curve::ReceivedBadBytes]->attach(this);
                    _curves[Curve::ReceivedBadBytes]->setStyle(QPen(Qt::red, 1.0, Qt::DashDotDotLine), QColor(255, 255, 255, 0));

                    canvas->setCursor(QCursor());

                    for (size_t i = 0; i < HISTORY; ++i)
                    {
                        _timeData[HISTORY - 1 - i] = i;
                    }

                    _detailsText = new QwtText();
                    _detailsText->setRenderFlags(Qt::AlignTop | Qt::AlignLeft);
                    QFont font = _detailsText->font();
                    font.setPointSize(7);
                    _detailsText->setFont(font);

                    _detailsLabel = new QwtPlotTextLabel();
                    _detailsLabel->setText(*_detailsText);
                    _detailsLabel->attach(this);

                    _timerId = startTimer(1000 / REFRESH_RATE);
                }

                ~NetStatisticsWidget()
                {
                    killTimer(_timerId);
                    delete _detailsText;
                }

                void updateStats(mcc::misc::NetStatistics&& stats)
                {
                    _stats = stats;
                }

                double sentBytesSpeed()          const { return _curves[Curve::SentBytes]->speed();         }
                double sentPacketsSpeed()        const { return _curves[Curve::SentPackets]->speed();       }
                double receivedBytesSpeed()      const { return _curves[Curve::ReceivedBytes]->speed();     }
                double receivedPacketsSpeed()    const { return _curves[Curve::ReceivedPackets]->speed();   }
                double receivedBadBytesSpeed()   const { return _curves[Curve::ReceivedBadBytes]->speed();  }
                double receivedBadPacketsSpeed() const { return _curves[Curve::ReceivedBadPackets]->speed();}

                static QString formatSpeed(double speed)
                {
                    if (speed > 1024 * 1024)
                        return QString("%1 Мб/с").arg(speed / 1024 * 1024, 2, 'f', 2);
                    else if (speed > 1024)
                        return QString("%1 Кб/с").arg(speed / 1024, 2, 'f', 2);

                    return QString("%1 б/с").arg(speed, 2, 'f', 2);
                }

                void showDetails(bool showDetails)
                {
                    _detailsLabel->setVisible(showDetails);
                }
            private:
                virtual void timerEvent(QTimerEvent *) override
                {
                    for (auto& time : _timeData)
                        time++;

                    for (auto& curve : _curves)
                        curve->shiftLeft();

                    _curves[Curve::SentBytes]->update(_stats._sent, _stats._sentBytes);
                    _curves[Curve::ReceivedBytes]->update(_stats._rcvd, _stats._rcvdBytes);
                    _curves[Curve::ReceivedBadBytes]->update(_stats._rcvdBad, _stats._rcvdBadBytes);

                    _curves[Curve::SentPackets]->update(_stats._sent, _stats._sentPackets);
                    _curves[Curve::ReceivedPackets]->update(_stats._rcvd, _stats._rcvdPackets);
                    _curves[Curve::ReceivedBadPackets]->update(_stats._rcvdBad, _stats._rcvdBadPackets);

                    rescalePlot();

                    for (auto& curve : _curves)
                        curve->setTimeData(_timeData.data());

                    double sentSpeed = _curves[Curve::SentBytes]->speed();
                    double recvSpeed = _curves[Curve::ReceivedBytes]->speed();

                    QString detailsText = QString("%1\n%2").arg(NetStatisticsWidget::formatSpeed(sentSpeed)).arg(NetStatisticsWidget::formatSpeed(recvSpeed));

                    _detailsText->setText(detailsText);
                    _detailsLabel->setText(*_detailsText);
                    replot();
                }

                void rescalePlot()
                {
                    setAxisScale(QwtPlot::xBottom, _timeData[HISTORY - 1], _timeData[0]);

                    unsigned int maxValue = 0;

                    for (const auto& curve : _curves)
                    {
                        auto maxElement = curve->maxElement();
                        if (maxElement > maxValue)
                            maxValue = maxElement;
                    }

                    maxValue = qRound(maxValue * 1.1);

                    if (maxValue != _maxValue)
                    {
                        _maxValue = maxValue;
                        setAxisScale(QwtPlot::yLeft, 0, _maxValue, 1.0);
                    }
                }
            private:
                int                                _timerId;
                double                             _maxValue;
                mcc::misc::NetStatistics           _stats;
                std::array<double, HISTORY>        _timeData;
                std::array<CurvePtr, Curve::Count> _curves;
                QString                            _tooltipText;
                QwtText*                           _detailsText;
                QwtPlotTextLabel*                  _detailsLabel;
            };

        }
    }
}

