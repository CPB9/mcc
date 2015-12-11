/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#pragma once

#include <qwt_plot.h>
#include <qwt_plot_curve.h>
#include <qwt_plot_canvas.h>
#include <qwt_legend.h>

class LegendItem;

class SignalBar : public QwtPlot
{
    Q_OBJECT

public:
    explicit SignalBar(QWidget* parent = 0);

    void setValue(double value);

    virtual void timerEvent(QTimerEvent *) override;

    void setEnabled(bool flag);

    void restoreCanvasBorder();
private:
    bool _isEnabled;

    int _historyTicks;

    double _value;

    QVector<double> _data;
    QVector<double> _time;

    QwtPlotCurve* _curve;
    QwtPlotCanvas* _canvas;
    LegendItem*    _legend;
};

