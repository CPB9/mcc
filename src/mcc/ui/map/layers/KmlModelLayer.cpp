/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "mcc/ui/map/layers/KmlModelLayer.h"
#include "mcc/ui/map/layers/KmlPointLayer.h"
#include "mcc/ui/map/layers/KmlPolyLineLayer.h"
#include "mcc/ui/map/layers/KmlPolygonLayer.h"
#include "mcc/ui/map/KmlModel.h"
#include "mcc/ui/map/KmlUtils.h"

#include <QAction>
#include <QDebug>
#include <QPainter>

namespace mcc {
namespace ui {
namespace map {

std::shared_ptr<KmlElementLayer>
KmlModelLayer::createPlacemark(kmldom::Document* parent, const kmldom::PlacemarkPtr& pm, const MapRectConstPtr& rect)
{
    kmldom::GeometryPtr geometry = pm->get_geometry();
    kmldom::StylePtr style = KmlUtils::resolveStyle(parent, pm.get());
    kmldom::CoordinatesPtr coordinates = KmlUtils::resolveCoordinates(pm.get());
    QString label = QString::fromStdString(pm->get_name());
    if (geometry->IsA(kmldom::Type_Point)) {
        return std::make_shared<KmlPointLayer>(label, coordinates, style, rect);
    } else if (geometry->IsA(kmldom::Type_LineString)) {
        return std::make_shared<KmlPolyLineLayer>(label, coordinates, style, rect);
    } else if (geometry->IsA(kmldom::Type_Polygon)) {
        return std::make_shared<KmlPolygonLayer>(label, coordinates, style, rect);
    }
    return nullptr;
}

void KmlModelLayer::addElement(kmldom::Document* parent, const kmldom::ElementPtr& element, bool isActive)
{
    if (element->IsA(kmldom::Type_Placemark)) {
        kmldom::PlacemarkPtr pm = kmldom::AsPlacemark(element);
        std::shared_ptr<KmlElementLayer> layer = createPlacemark(parent, pm, mapRect());
        if (layer) {
            layer->setActive(isActive);
            if (element == _currentSelected) {
                layer->setActive(true);
                _currentLayer = layer;
            }
            _layers[pm] = std::move(layer);
        }
    }
}

KmlModelLayer::KmlModelLayer(KmlModel* model, const MapRectConstPtr& rect)
    : Layer(rect)
    , _model(model)
    , _currentSelected(0)
    , _currentLayer(0)
{
    kmldom::ElementPtr root = model->root();
    kmldom::DocumentPtr doc = _model->rootDocument();
    initFrom(doc.get(), root);
    connect(_model, &KmlModel::modelReset, this, [this]() {
        _layers.clear();
        kmldom::ElementPtr root = _model->root();
        kmldom::DocumentPtr doc = _model->rootDocument();
        initFrom(doc.get(), root);
        emit sceneUpdated();
    });
    connect(_model, &KmlModel::visibilityChanged, this,
            [this](kmldom::Document* parent, const kmldom::ElementPtr& element, bool isVisible, bool isSelected) {
                if (!isVisible) {
                    auto it = _layers.find(element);
                    if (it != _layers.end()) {
                        _layers.erase(it);
                    }
                } else {
                    addElement(parent, element, isSelected);
                }
                emit sceneUpdated();
            });
    connect(_model, &KmlModel::elementAdded, this, [this](kmldom::Document* parent, const kmldom::ElementPtr& element) {
        addElement(parent, element);
        emit sceneUpdated();
    });
    connect(_model, &KmlModel::elementChanged, this,
            [this](kmldom::Document* parent, const kmldom::ElementPtr& element) {
                auto it = _layers.find(element);
                if (it != _layers.end()) {
                    _layers.erase(it);
                    addElement(parent, element);
                }
                emit sceneUpdated();
            });
    connect(_model, &KmlModel::elementRemoved, this, [this](kmldom::Document* parent, kmldom::Element* element) {
        (void)parent;
        auto it = _layers.find(element);
        if (it != _layers.end()) {
            _layers.erase(it);
            if (_currentSelected == element) {
                _currentLayer.reset();
                _currentSelected.reset();
            }
        }
        emit sceneUpdated();
    });
    connect(_model, &KmlModel::elementSelected, this, [this](kmldom::Document* parent, kmldom::Element* element) {
        (void)parent;
        auto it = _layers.find(element);
        if (_currentSelected) {
            _currentLayer->setActive(false);
        }
        if (it != _layers.end()) {
            _currentSelected = it->first;
            _currentLayer = it->second;
            _currentLayer->setActive(true);
        } else {
            _currentSelected.reset();
            _currentLayer.reset();
        }
        emit sceneUpdated();
    });
}

KmlModelLayer::~KmlModelLayer()
{
}

void KmlModelLayer::initFrom(kmldom::Document* parent, const kmldom::ElementPtr& element)
{
    if (element->IsA(kmldom::Type_Container)) {
        if (element->IsA(kmldom::Type_Document)) {
            parent = static_cast<kmldom::Document*>(element.get());
        }
        kmldom::Container* container = static_cast<kmldom::Container*>(element.get());
        for (std::size_t i = 0; i < container->get_feature_array_size(); i++) {
            kmldom::FeaturePtr feature = container->get_feature_array_at(i).get();
            initFrom(parent, feature);
        }
    } else {
        addElement(parent, element);
    }
}

void KmlModelLayer::draw(QPainter* p) const
{
    for (const auto& pair : _layers) {
        pair.second->draw(p);
    }
}

template <typename F, typename... A>
void KmlModelLayer::visitLayers(F func, A&&... args)
{
    for (auto& pair : _layers) {
        (pair.second.get()->*func)(std::forward<A>(args)...);
    }
}

template <typename F, typename... A>
bool KmlModelLayer::visitCurrentLayer(F func, A&&... args)
{
    if (_currentLayer) {
        return (_currentLayer.get()->*func)(std::forward<A>(args)...);
    }
    return false;
}

template <typename F, typename... A>
void KmlModelLayer::visitLayersUntilAccepts(F func, A&&... args)
{
    for (auto& pair : _layers) {
        if ((pair.second.get()->*func)(std::forward<A>(args)...)) {
            return;
        }
    }
}

void KmlModelLayer::changeProjection(const MercatorProjection& from, const MercatorProjection& to)
{
    visitLayers(&Layer::changeProjection, from, to);
}

bool KmlModelLayer::contextMenuEvent(const QPoint& pos)
{
    return visitCurrentLayer(&Layer::contextMenuEvent, pos);
}

bool KmlModelLayer::mouseMoveEvent(const QPoint& oldPos, const QPoint& newPos)
{
    return visitCurrentLayer(&Layer::mouseMoveEvent, oldPos, newPos);
}

bool KmlModelLayer::mousePressEvent(const QPoint& pos)
{
    if (!visitCurrentLayer(&Layer::mousePressEvent, pos)) {
        for (auto& pair : _layers) {
            if (pair.second.get()->hasElementAt(pos)) {
                if (_currentLayer) {
                    _currentLayer->setActive(false);
                }
                pair.second->setActive(true);
                _currentLayer = pair.second;
                _currentSelected = pair.first;
                emit sceneUpdated();
                return _currentLayer->mousePressEvent(pos);
            }
        }
    } else {
        return true;
    }
    return false;
}

bool KmlModelLayer::mouseReleaseEvent(const QPoint& pos)
{
    return visitCurrentLayer(&Layer::mouseReleaseEvent, pos);
}

bool KmlModelLayer::viewportResetEvent(int oldZoom, int newZoom, const QRect& oldViewpiort, const QRect& newViewport)
{
    visitLayers(&Layer::viewportResetEvent, oldZoom, newZoom, oldViewpiort, newViewport);
    return true;
}

bool KmlModelLayer::viewportResizeEvent(const QSize& oldSize, const QSize& newSize)
{
    visitLayers(&Layer::viewportResizeEvent, oldSize, newSize);
    return true;
}

bool KmlModelLayer::viewportScrollEvent(const QPoint& oldPos, const QPoint& newPos)
{
    visitLayers(&Layer::viewportScrollEvent, oldPos, newPos);
    return true;
}

bool KmlModelLayer::zoomEvent(const QPoint& pos, int fromZoom, int toZoom)
{
    visitLayers(&Layer::zoomEvent, pos, fromZoom, toZoom);
    return true;
}
}
}
}
