/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "mcc/ui/map/drawables/RulerLabel.h"
#include "mcc/ui/map/drawables/Label.h"

#include <QPainter>

#include <cmath>

namespace mcc {
namespace ui {
namespace map {

void RulerLabel::draw(QPainter* p, const MapRectConstPtr& rect) const
{
    if (!_isVisible) {
        return;
    }
    QTransform t = p->transform();
    // p->rotate(_rotation);
    p->drawPixmap(_point.position() + _offset - rect->mapOffsetRaw(), _pixmap);
    p->setTransform(t);
}

void RulerLabel::setDistanceAndRotation(double distance, double rotation)
{
    _rotation = rotation;
    const char* suffix = "м";
    if (distance > 1000) {
        distance /= 1000;
        suffix = "км";
    }
    QString az = core::CoordinatePrinter().printValue(std::fmod(rotation + 360, 360), 2);
    if (rotation > 90) {
        rotation -= 180;
    } else if (rotation < -90) {
        rotation += 180;
    }
    QString text = (QString("%1%2 %3")).arg(distance, 0, 'f', 2).arg(suffix).arg(az);
    QTransform t;
    t.rotate(-rotation);
    double radians = qDegreesToRadians(-rotation + 90);
    QImage img = Label::createLabelImage(text, Qt::white);
    int labelOffset = img.height() * 1.1;
    _pixmap = QPixmap::fromImage(img.transformed(t, Qt::SmoothTransformation));
    _offset = -_pixmap.rect().center() - labelOffset * QPointF(std::cos(radians), std::sin(radians));
}
}
}
}
