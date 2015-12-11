/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "SignalBar.h"

#include <qwt_plot_curve.h>
#include <qwt_plot_legenditem.h>
#include <qwt_legend.h>
#include <qwt_plot_grid.h>
#include <qwt_plot_layout.h>


class LegendItem : public QwtPlotLegendItem
{
public:
    LegendItem()
    {
        setRenderHint(QwtPlotItem::RenderAntialiased);

        QColor color(Qt::white);

        setTextPen(color);
        setBorderPen(color);

        QColor c(Qt::gray);
        c.setAlpha(200);

        setBackgroundBrush(c);
    }
};

SignalBar::SignalBar(QWidget* parent)
    : QwtPlot(parent)
    , _isEnabled(false)
    , _historyTicks(100)
    , _value(0.0)
{
    setAutoReplot(false);

    _canvas = new QwtPlotCanvas();
    restoreCanvasBorder();

    plotLayout()->setAlignCanvasToScales(true);

    setAxisScale(QwtPlot::yLeft, 0.1, 100);
    setAxisScale(QwtPlot::xBottom, -10, 0);

    enableAxis(QwtPlot::yLeft, false);
    enableAxis(QwtPlot::xBottom, false);

    _curve = new QwtPlotCurve();
    _curve->attach(this);
    _curve->setBrush(Qt::darkYellow);
    _curve->setLegendIconSize(QSize(0, 0));

    _legend = new LegendItem();
    _legend->attach(this);

    _legend->setMaxColumns(1);
    _legend->setAlignment(Qt::AlignLeft | Qt::AlignTop);

    _time.append(0.1);
    _data.append(0.0);

    this->canvas()->setCursor(QCursor());

    startTimer(100);
}

void SignalBar::setValue(double value)
{
    _curve->setTitle(QString::number(value, 'f', 1));
    _value = value;
}

void SignalBar::timerEvent(QTimerEvent *)
{
    if (!_isEnabled)
        return;

     _data.prepend(_value);
     _time.prepend(0.2);

     for (int i = 0; i < _time.count(); ++i)
     {
         _time[i] -= 0.1;
     }

     if (_data.count() > _historyTicks)
     {
         _data.removeLast();
         _time.removeLast();
     }

     _curve->setRawSamples(_time.data(), _data.data(), _data.size());

     replot();
}

void SignalBar::setEnabled(bool flag)
{
    _isEnabled = flag;
}

void SignalBar::restoreCanvasBorder()
{
    _canvas->setFrameStyle(QFrame::Box | QFrame::Plain);
    _canvas->setLineWidth(1);
    _canvas->setContentsMargins(0, 0, 0, 0);

//    setStyleSheet("border:1px solid;");
    setCanvas(_canvas);
}
