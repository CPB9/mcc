/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#pragma once

#include "mcc/ui/map/drawables/Point.h"
#include "mcc/ui/map/drawables/Marker.h"
#include "mcc/ui/map/drawables/Label.h"

#include <QPainter>
#include <QPixmap>
#include <QImage>

#include <QString>

#include <functional>

namespace mcc {
namespace ui {
namespace map {

class Flag : public AbstractMarker<Flag> {
public:
    static QImage drawFlag(quint32 width, quint32 height, QColor color);
    static QImage drawWaypointFlag(quint32 width, quint32 height, QColor color, quint32 waypointNumber,
                                   quint32 waypointFlags = 0);
    static QImage drawWaypointFlag(quint32 width, quint32 height, QColor color);

    Flag(const QPointF& position, const QPixmap& active, const QPixmap& inactive, const QString& name,
         const MapRectConstPtr& rect);
    Flag(const MapRectConstPtr& rect);

    void draw(QPainter* p, const MapRectConstPtr& rect) const;
    QRectF rect() const;
    void moveBy(const QPointF& delta);
    void changeZoomLevel(int from, int to);
    void drawWithoutLabel(QPainter* p, const MapRectConstPtr& rect) const;
    void drawLabel(QPainter* p, const MapRectConstPtr& rect) const;

    void setPosition(const QPointF& position);
    const QPointF& position() const;

    void setLabelBackground(const QColor& color);
    void setLabelScale(double scale);
    void setActivePixmap(const QPixmap& p);
    void setInactivePixmap(const QPixmap& p);
    void setLabel(const QString& label);
    bool isActive() const;
    void setActive(bool isActive);
    const QString& name() const;
    void setName(const QString& name);
    double labelScale() const;
    void updateLabel();

private:
    QString _name;
    LabelBase _label;
    MarkerBase _activeMarker;
    MarkerBase _inactiveMarker;
    Point _point;
    bool _isActive;
    MapRectConstPtr _rect;


    static void drawFlag(QPainter& painter, const QRectF& rect);
};

inline const QPointF& Flag::position() const
{
    return _point.position();
}

inline double Flag::labelScale() const
{
    return _label.labelScale();
}
}
}
}
