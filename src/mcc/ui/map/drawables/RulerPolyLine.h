/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#pragma once

#include "mcc/ui/map/drawables/PolyLine.h"
#include "mcc/ui/map/drawables/RulerLabel.h"

namespace mcc {
namespace ui {
namespace map {

template <typename T>
class RulerPolyLine {
public:
    RulerPolyLine(const MapRectConstPtr& rect)
        : _points(rect)
        , _isConnected(false)
    {
    }

    core::GeoBox geoBox() const
    {
        return _points.geoBox();
    }

    void setConnected(bool isConnected)
    {
        _isConnected = isConnected;
        updateArrowheads();
    }

    double distanceTo(std::size_t index) const
    {
        return _points.distanceTo(index);
    }

    double distanceTo(std::size_t index, const QPointF& next) const
    {
        return _points.distanceTo(index, next);
    }

    void swap(std::size_t first, std::size_t second)
    {
        _points.swap(first, second);
        updateLabelsAround(first);
        updateLabelsAround(second);
    }

    void setPosition(std::size_t index, const QPointF& pos)
    {
        _points.setPosition(index, pos);
        updateLabelsAround(index);
    }

    void setLatLon(std::size_t index, const core::LatLon& latLon)
    {
        _points.setLatLon(index, latLon);
        updateLabelsAround(index);
    }

    void setLineWidth(double width)
    {
        _points.setLineWidth(width);
        updateArrowheads();
    }

    void setLineColor(const QColor& color)
    {
        _points.setLineColor(color);
        updateArrowheads();
    }

    void clear()
    {
        _points.clear();
        _labels.clear();
        _arrowheads = QPainterPath();
    }

    double totalDistance() const
    {
        return _points.totalDistance(_isConnected);
    }

    std::size_t count() const
    {
        return _points.count();
    }

    void remove(std::size_t index)
    {
        _labels.erase(_labels.begin() + index);
        _points.remove(index);
        updateLabelsAround(index);
    }

    const typename PolyLineBase<T>::ElementType& at(std::size_t index) const
    {
        return _points.at(index);
    }

    typename PolyLineBase<T>::ElementType& at(std::size_t index)
    {
        return _points.at(index);
    }

    void changeProjection(const MercatorProjection& from, const MercatorProjection& to)
    {
        _points.changeProjection(from, to);
        visitLabels(&WithPosition<RulerLabel>::changeProjection, _points.mapRect(), from, to);
        updateArrowheads();
    }

    void changeZoomLevel(int from, int to)
    {
        _points.changeZoomLevel(from, to);
        visitLabels(&Zoomable<RulerLabel>::changeZoomLevel, from, to);
        updateArrowheads();
    }

    misc::Option<std::size_t> nearest(const QPointF& position) const
    {
        return _points.nearest(position);
    }

    misc::Option<LineIndexAndPos> nearestLine(const QPointF& position, int width) const
    {
        return _points.nearestLine(position, width, _isConnected);
    }

    void draw(QPainter* p) const
    {
        if (_isConnected) {
            _points.drawLinesConnected(p);
        } else {
            _points.drawLines(p);
        }
        drawArrowheads(p);
        drawLabels(p);
        _points.drawPoints(p);
    }

    void drawConnected(QPainter* p) const
    {
        _points.drawLinesConnected(p);
        drawArrowheads(p);
        drawLabels(p);
        _points.drawPoints(p);
    }

    template <typename... A>
    void emplace(std::size_t index, A&&... args)
    {
        _points.emplace(index, std::forward<A>(args)...);
        createLabelAfterInsert(index);
    }

    void insert(std::size_t index, const T& point)
    {
        _points.insert(index, point);
        createLabelAfterInsert(index);
    }

    void insert(std::size_t index, T&& point)
    {
        _points.insert(index, std::forward<T>(point));
        createLabelAfterInsert(index);
    }

    void append(const T& point)
    {
        _points.append(point);
        createLabelAfterInsert(_points.count() - 1);
    }

    void append(T&& point)
    {
        _points.append(std::forward<T>(point));
        createLabelAfterInsert(_points.count() - 1);
    }

    void moveBy(std::size_t index, const QPointF& pos)
    {
        _points.moveBy(index, pos);
        updateLabelsAround(index);
    }

private:
    void drawLabels(QPainter* p) const
    {
        if (_labels.size() > 1) {
            auto end = _labels.end();
            if (!_isConnected) {
                end--;
            }
            for (auto it = _labels.begin(); it < end; it++) {
                it->draw(p, _points.mapRect());
            }
        }
    }

    void updateArrowheads()
    {
        _arrowheads = _points.createArrowheads(_isConnected);
    }

    void drawArrowheads(QPainter* p) const
    {
        QPen pen;
        pen.setWidthF(_points.lineWidth());
        pen.setColor(_points.lineColor());
        p->strokePath(_arrowheads.translated(-_points.mapRect()->mapOffsetRaw()), pen);
    }

    void updateVisibility(std::size_t index, std::size_t nextIndex)
    {
        auto it = _points.points().begin() + index;
        RulerLabel& label = _labels.at(index);
        double labelHypot = std::hypot(label.rect().width(), label.rect().height());
        QPointF mapDiff = it->position() - (_points.points().begin() + nextIndex)->position();
        double mapHypot = std::hypot(mapDiff.x(), mapDiff.y());
        label.setVisible(labelHypot < mapHypot);
    }

    void updateLabelsAround(std::size_t index)
    {
        updateArrowheads();
        std::size_t lastIndex = _points.count() - 1;
        if (_points.count() < 2) {
            return;
        } else if (index > lastIndex) {
            index = lastIndex;
        }
        auto update = [this](std::size_t index, std::size_t nextIndex) {
            auto it = _points.points().begin() + index;
            RulerLabel& label = _labels.at(index);
            updateVisibility(index, nextIndex);
            label.setDistanceAndRotation(it->distance(), it->angle());
            label.setPosition(_points.positionBetween(index, nextIndex));
        };
        if (index == 0) {
            update(lastIndex, 0);
        } else {
            update(index - 1, index);
        }
        if (index == lastIndex) {
            update(lastIndex, 0);
        } else {
            update(index, index + 1);
        }
    }

    void createLabelAfterInsert(std::size_t index)
    {
        _labels.emplace(_labels.begin() + index, QPoint(0, 0), 0, 0);
        updateLabelsAround(index); // TODO: обновлять только 1 точку вместо 2
    }

    template <typename F, typename... A>
    void visitLabels(F func, A&&... args)
    {
        if (_labels.size() > 0) {
            for (std::size_t i = 0; i < _labels.size() - 1; i++) {
                RulerLabel& label = _labels.at(i);
                (label.*func)(std::forward<A>(args)...);
                updateVisibility(i, i + 1);
            }
            updateVisibility(_labels.size() - 1, 0);
        }
    }

    QPainterPath _arrowheads;
    PolyLineBase<T> _points;
    std::vector<RulerLabel> _labels;
    bool _isConnected;
};
}
}
}
