/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "mcc/ui/map/layers/Layer.h"
#include "mcc/ui/map/drawables/WithPosition.h"
#include "mcc/ui/map/drawables/Label.h"
#include "mcc/ui/core/CoordinatePrinter.h"
#include "mcc/ui/core/GlobalCoordinatePrinter.h"
#include "mcc/ui/core/Structs.h"

#include <QPointF>
#include <QWidget>

namespace mcc {
namespace ui {
namespace map {

Layer::~Layer()
{
}

void Layer::drawCoordinatesAt(QPainter* p, const QPointF& coord, const QPointF& point, const QString& arg, double scale) const
{
    core::LatLon latLon = WithPosition<>::toLatLon(coord, mapRect());
    QString str = core::GlobalCoordinatePrinter::instance()->printer().print(latLon.latitude, latLon.longitude, arg);
    Label::drawLabelAt(p, str, point - _rect->mapOffsetRaw(), Qt::yellow, Qt::AlignBottom | Qt::AlignLeft, scale);
}

void Layer::setCursor(const QCursor& cursor)
{
    _rect->parent()->setCursor(cursor);
}
}
}
}
