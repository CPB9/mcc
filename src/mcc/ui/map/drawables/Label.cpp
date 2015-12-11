/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "mcc/ui/map/drawables/Label.h"
#include "mcc/ui/core/GlobalCoordinatePrinter.h"

#include <QImage>
#include <QApplication>

namespace mcc {
namespace ui {
namespace map {

QSize LabelBase::computeSize(const QString& text)
{
    return QFontMetrics(QApplication::font()).size(0, text);
}

QImage LabelBase::createLabelImage(const QString& text, const QColor& color)
{
    QPainter p;
    QSize size = QFontMetrics(QApplication::font()).size(0, text);
    QImage img(size, QImage::Format_ARGB32_Premultiplied);
    p.begin(&img);
    QRectF rect(QPoint(0, 0), size);
    p.fillRect(rect, color);
    p.drawText(rect, text);
    p.end();
    return img;
}

void LabelBase::repaintLabelPixmap()
{
    QPainter p;
    QFont font = QApplication::font();
    font.setPointSizeF(font.pointSizeF() * _labelScale);
    QSize size = QFontMetrics(font).size(0, _label);
    QImage img(size, QImage::Format_ARGB32_Premultiplied);
    img.fill(Qt::transparent);
    p.begin(&img);
    p.setFont(font);
    QRectF rect(QPoint(0, 0), size);
    p.fillRect(rect, _background);
    p.drawText(rect, _label);
    _renderedLabel = QPixmap::fromImage(img);
    _offset = WithPosition<>::edgePoint(img.size(), _alignment);
    p.end();
}

void LabelBase::drawLabelAt(QPainter* p, const QString& text, const QPointF& point, const QColor& color)
{
    QSize size = QFontMetrics(QApplication::font()).size(0, text);
    QRectF rect(point, size);
    p->fillRect(rect, color);
    p->drawText(rect, text);
}

void LabelBase::drawLabelAt(QPainter* p, const QString& text, const QPointF& point, const QColor& color,
                            Qt::Alignment alignment, double scale)
{
    QFont oldFont = p->font();
    QFont font = QApplication::font();
    font.setPointSizeF(font.pointSizeF() * scale);
    QSize size = QFontMetrics(font).size(0, text);
    QPointF offset = WithPosition<>::edgePoint(size, alignment);
    QRectF rect(point + offset, size);
    p->setFont(font);
    p->fillRect(rect, color);
    p->drawText(rect, text);
    p->setFont(oldFont);
}
}
}
}
