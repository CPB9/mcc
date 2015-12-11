/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#pragma once

#include "mcc/ui/map/drawables/Drawable.h"
#include "mcc/ui/map/Ptr.h"

#include <QObject>

class QPainter;
class QPoint;
class QSize;
class QString;
class QCursor;

namespace mcc {
namespace ui {
namespace map {

class MercatorProjection;

class Layer : public QObject {
    Q_OBJECT
public:
    Layer(const MapRectConstPtr& rect);

    virtual ~Layer();
    virtual void draw(QPainter* p) const = 0;
    virtual bool mousePressEvent(const QPoint& pos) = 0;
    virtual bool mouseReleaseEvent(const QPoint& pos) = 0;
    virtual bool mouseMoveEvent(const QPoint& oldPos, const QPoint& newPos) = 0;
    virtual bool viewportResizeEvent(const QSize& oldSize, const QSize& newSize) = 0;
    virtual bool viewportScrollEvent(const QPoint& oldPos, const QPoint& newPos) = 0;
    virtual bool contextMenuEvent(const QPoint& pos) = 0;
    virtual bool zoomEvent(const QPoint& pos, int fromZoom, int toZoom) = 0;
    virtual bool viewportResetEvent(int oldZoom, int newZoom, const QRect& oldViewpiort, const QRect& newViewport) = 0;

    void drawCoordinatesAt(QPainter* p, const QPointF& coord, const QPointF& point,
                           const QString& arg = "%1, %2", double scale = 1) const;
    const MapRectConstPtr& mapRect() const;

public slots:
    virtual void changeProjection(const MercatorProjection& from, const MercatorProjection& to) = 0;

signals:
    void sceneUpdated();

protected:
    void setCursor(const QCursor& cursor);

private:
    MapRectConstPtr _rect;
};

inline Layer::Layer(const MapRectConstPtr& rect)
    : _rect(rect)
{
}

inline const MapRectConstPtr& Layer::mapRect() const
{
    return _rect;
}
}
}
}
