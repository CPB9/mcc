/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#pragma once

#include "mcc/ui/map/drawables/Interfaces.h"
#include "mcc/ui/map/drawables/Point.h"

#include <QPainter>
#include <QString>
#include <QColor>
#include <QPointF>
#include <QPixmap>

namespace mcc {
namespace ui {
namespace map {

class LabelBase {
public:
    LabelBase(const QString& label = QString());

    void drawAt(QPainter* p, const QPointF& pos) const;
    static QImage createLabelImage(const QString& text, const QColor& color);
    static void drawLabelAt(QPainter* p, const QString& text, const QPointF& point, const QColor& color);
    static void drawLabelAt(QPainter* p, const QString& text, const QPointF& point, const QColor& color,
                            Qt::Alignment alignment, double scale = 1);
    static QSize computeSize(const QString& text);
    void setLabel(const QString& label);
    const QString& label() const;
    void setLabelBackground(const QColor& color);
    void setLabelScale(double scale);
    void setLabelParams(const QString& label, const QColor& color, double scale);
    void setLabelAlignment(Qt::Alignment alignment);
    double labelScale() const;

protected:
    const QPixmap& labelPixmap() const;
    void repaintLabelPixmap();

    QString _label;
    QColor _background;
    QPointF _offset;
    QPixmap _renderedLabel;
    double _labelScale;
    Qt::Alignment _alignment;
};

inline LabelBase::LabelBase(const QString& label)
    : _label(label)
    , _background(Qt::white)
    , _labelScale(1)
    , _alignment(Qt::AlignCenter)
{
}

inline void LabelBase::drawAt(QPainter* p, const QPointF& pos) const
{
    p->drawPixmap(pos + _offset, _renderedLabel);
}

inline const QString& LabelBase::label() const
{
    return _label;
}

inline const QPixmap& LabelBase::labelPixmap() const
{
    return _renderedLabel;
}

inline void LabelBase::setLabel(const QString& label)
{
    _label = label;
    repaintLabelPixmap();
}

inline void LabelBase::setLabelScale(double scale)
{
    _labelScale = scale;
    repaintLabelPixmap();
}

inline void LabelBase::setLabelBackground(const QColor& color)
{
    _background = color;
    repaintLabelPixmap();
}

inline void LabelBase::setLabelParams(const QString& label, const QColor& color, double scale)
{
    _label = label;
    _background = color;
    _labelScale = scale;
    repaintLabelPixmap();
}

inline void LabelBase::setLabelAlignment(Qt::Alignment alignment)
{
    _alignment = alignment;
    repaintLabelPixmap();
}

class Label : public AbstractMarker<Label>, public LabelBase {
public:
    Label(const QPointF& position = QPointF(0, 0), const QString& label = QString());

    void draw(QPainter* p, const MapRectConstPtr& rect) const;
    QRectF rect() const;
    void moveBy(const QPointF& delta);
    void changeZoomLevel(int from, int to);

    const QPointF& position() const;
    void setPosition(const QPointF& position);

private:
    Point _point;
};

inline void Label::draw(QPainter* p, const MapRectConstPtr& rect) const
{
    p->drawPixmap(_point.position() + _offset - rect->mapOffsetRaw(), labelPixmap());
}

inline void Label::changeZoomLevel(int from, int to)
{
    _point.changeZoomLevel(from, to);
}

inline void Label::moveBy(const QPointF& delta)
{
    _point.moveBy(delta);
}

inline QRectF Label::rect() const
{
    return WithRect::positionedRect(labelPixmap().rect(), _point.position());
}

inline const QPointF& Label::position() const
{
    return _point.position();
}

inline void Label::setPosition(const QPointF& position)
{
    _point.setPosition(position);
}

inline Label::Label(const QPointF& position, const QString& label)
    : LabelBase(label)
    , _point(position)
{
}

inline double LabelBase::labelScale() const
{
    return _labelScale;
}
}
}
}
