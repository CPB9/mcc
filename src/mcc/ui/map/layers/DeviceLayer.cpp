/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "mcc/ui/map/layers/DeviceLayer.h"
#include "mcc/ui/map/layers/AircraftLayer.h"
#include "mcc/ui/map/layers/WaypointLayer.h"
#include "mcc/ui/map/layers/TemplateLayer.h"
#include "mcc/ui/core/DeviceManager.h"
#include "mcc/ui/core/FlyingDevice.h"
#include "mcc/ui/map/MapRect.h"

#include <QPainter>
#include <QAction>
#include <QMenu>

namespace mcc {
namespace ui {
namespace map {

using core::DeviceManager;
using core::FlyingDevice;

class AircraftWaypointsPair : public QObject {
public:
    static std::unique_ptr<AircraftWaypointsPair> create(core::FlyingDevice* device, DeviceLayer* parent);
    AircraftWaypointsPair(const AircraftWaypointsPair& other) = delete;
    AircraftWaypointsPair(AircraftWaypointsPair&& other) = delete;
    AircraftWaypointsPair& operator=(const AircraftWaypointsPair& other) = delete;
    AircraftWaypointsPair& operator=(AircraftWaypointsPair&& other) = delete;

    void draw(QPainter* p) const;

    template <typename F, typename... A>
    void mapWaypointLayers(F&& func, A&&... args);
    template <typename F, typename... A>
    void mapWaypointLayers(F&& func, A&&... args) const;

    const std::unique_ptr<AircraftLayer>& aircraft();
    core::FlyingDevice* device();
    const core::FlyingDevice* device() const;

private:
    AircraftWaypointsPair(core::FlyingDevice* device, DeviceLayer* parent);
    core::FlyingDevice* _device;
    std::unique_ptr<AircraftLayer> _alayer;
    std::map<core::Route*, std::unique_ptr<WaypointLayer>> _wlayers;
};

template <typename F, typename... A>
inline void AircraftWaypointsPair::mapWaypointLayers(F&& func, A&&... args)
{
    for (const auto& pair : _wlayers) {
        (pair.second.get()->*func)(std::forward<A>(args)...);
    }
}

template <typename F, typename... A>
inline void AircraftWaypointsPair::mapWaypointLayers(F&& func, A&&... args) const
{
    for (const auto& pair : _wlayers) {
        (pair.second.get()->*func)(std::forward<A>(args)...);
    }
}

std::unique_ptr<AircraftWaypointsPair> AircraftWaypointsPair::create(FlyingDevice* device, DeviceLayer* parent)
{
    return std::unique_ptr<AircraftWaypointsPair>(new AircraftWaypointsPair(device, parent));
}

AircraftWaypointsPair::AircraftWaypointsPair(FlyingDevice* device, DeviceLayer* parent)
    : QObject(parent)
    , _device(device)
{
    _alayer.reset(new AircraftLayer(device, parent->mapRect()));
    connect(_alayer.get(), &AircraftLayer::sceneUpdated, parent, &DeviceLayer::sceneUpdated);

    auto addRoute = [this, parent, device](core::Route* route) {
        WaypointLayer* wlayer = new WaypointLayer(_device, route, parent->mapRect());
        wlayer->setStyle(WaypointLayer::Inactive);
        connect(wlayer, &WaypointLayer::sceneUpdated, parent, &DeviceLayer::sceneUpdated);
        _wlayers.emplace(route, std::unique_ptr<WaypointLayer>(wlayer));
    };

    for (core::Route* route : device->routes()) {
        addRoute(route);
    }

    connect(device, &FlyingDevice::routeAdded, this, addRoute);
    connect(device, &FlyingDevice::routeRemoved, this, [this](core::Route* route) { _wlayers.erase(route); });
}

void AircraftWaypointsPair::draw(QPainter* p) const
{
    mapWaypointLayers(&WaypointLayer::draw, p);
    _alayer->draw(p);
}

const std::unique_ptr<AircraftLayer>& AircraftWaypointsPair::aircraft()
{
    return _alayer;
}

const FlyingDevice* AircraftWaypointsPair::device() const
{
    return _device;
}

FlyingDevice* AircraftWaypointsPair::device()
{
    return _device;
}

void DeviceLayer::setActiveWaypointStyle(AircraftWaypointsPair* pair)
{
    pair->mapWaypointLayers(&WaypointLayer::setStyle, WaypointLayer::Active);
}

void DeviceLayer::setInactiveWaypointStyle(AircraftWaypointsPair* pair)
{
    pair->mapWaypointLayers(&WaypointLayer::setStyle, WaypointLayer::Inactive);
}

inline AircraftWaypointsPair* DeviceLayer::activeDevice() const
{
    if (_activeDevice.isSome())
        return _layers[_activeDevice.unwrap()].get();
    else
        return nullptr;
}

DeviceLayer::DeviceLayer(DeviceManager* manager, const MapRectConstPtr& rect)
    : Layer(rect)
    , _manager(manager)
    , _activeDevice(misc::None)
    , _isEditableLayerVisible(false)
{
    auto addDevice = [this](FlyingDevice* device) { _layers.push_back(AircraftWaypointsPair::create(device, this)); };

    _menu = new QMenu;
    _resetRouteAction = _menu->addAction("Сбросить");
    _resetRouteAction->setEnabled(false);
    connect(_resetRouteAction, &QAction::triggered, this, [this]() {
        activeDevice()->device()->resetEditableRoute(activeDevice()->device()->selectedRoute()->buffer()->id());
    });

    _clearRouteAction = _menu->addAction("Очистить");
    _clearRouteAction->setEnabled(false);
    connect(_clearRouteAction, &QAction::triggered, this, [this]() {
        activeDevice()->device()->findRouteById(activeDevice()->device()->selectedRoute()->buffer()->id())->buffer()->clear();
    });


    _menu->addSeparator();

    _applyRouteAction = _menu->addAction("На борт");
    _applyRouteAction->setEnabled(false);
    connect(_applyRouteAction, &QAction::triggered, this,
            [this]() { activeDevice()->device()->uploadEditableRoute(); });

    _menu->addSeparator();

    _endEditRouteAction = _menu->addAction("Выйти из редактирования");
    _endEditRouteAction->setEnabled(false);
    connect(_endEditRouteAction, &QAction::triggered, this,
         [this]() { emit activeDevice()->device()->routeUploaded(); });

    for (FlyingDevice* device : manager->devicesList()) {
        addDevice(device);
    }

    setCurrentDevice(manager->currentDevice());
    if (_activeDevice.isSome()) {
        setCurrentActive();
    }

    connect(manager, &core::DeviceManager::selectionChanged, this, [this](FlyingDevice* selected) {
        if (_activeDevice.isSome()) {
            _editableWaypointLayer.reset();
            activeDevice()->aircraft()->setActive(false);
            setInactiveWaypointStyle(activeDevice());
            disconnect(activeDevice()->device(), &FlyingDevice::positionChanged, this, 0);
        }
        setCurrentDevice(selected);
        setCurrentActive();
        emit sceneUpdated();
    });

    connect(manager, &core::DeviceManager::deviceAdded, this, [this, addDevice](FlyingDevice* device) {
        addDevice(device);
        reorderLayers();
        _resetRouteAction->setEnabled(_isEditableLayerVisible && _activeDevice.isSome());
        _clearRouteAction->setEnabled(_isEditableLayerVisible && _activeDevice.isSome());
        _applyRouteAction->setEnabled(_isEditableLayerVisible && _activeDevice.isSome());
    });

    connect(manager, &core::DeviceManager::beginEditTemplate, this, [this]() { setCurrentActive(true); });

    connect(manager, &core::DeviceManager::resetEditTemplate, this, [this]() { setCurrentActive(false); });

    connect(manager, &core::DeviceManager::endEditTemplate, this, [this](double delta, double height, double speed) {
        Layer* layer = _editableWaypointLayer.get();
        TemplateLayer* tlayer = dynamic_cast<TemplateLayer*>(layer);
        WaypointLayer* wlayer
            = tlayer->intoWaypointLayer(activeDevice()->device(), activeDevice()->device()->selectedRoute()->buffer(), delta, height, speed);
        wlayer->setStyle(WaypointLayer::Editable);
        setCurrentActive(wlayer);
    });

    connect(manager, &core::DeviceManager::deviceRemoved, this, [this](FlyingDevice* device) {
        auto activeDevice = this->activeDevice();

        if (activeDevice && (device == activeDevice->device())) {
            _editableWaypointLayer.reset();
            _activeDevice = misc::None;
            _resetRouteAction->setEnabled(false);
            _clearRouteAction->setEnabled(false);
            _applyRouteAction->setEnabled(false);
        }
        auto it = std::find_if(
            _layers.begin(), _layers.end(),
            [this, device](const std::unique_ptr<AircraftWaypointsPair>& pair) { return pair->device() == device; });
        _layers.erase(it);
    });
}

DeviceLayer::~DeviceLayer()
{
    delete _menu;
}

void DeviceLayer::setCurrentActive(Layer* layer)
{
    auto activeDevice = this->activeDevice();
    if (activeDevice == nullptr)
        return;

    setActiveWaypointStyle(activeDevice);
    _editableWaypointLayer.reset(layer);
    connect(layer, &Layer::sceneUpdated, this, &DeviceLayer::sceneUpdated);
    reorderLayers();
    _resetRouteAction->setEnabled(_isEditableLayerVisible && _activeDevice.isSome());
    _clearRouteAction->setEnabled(_isEditableLayerVisible && _activeDevice.isSome());
    _applyRouteAction->setEnabled(_isEditableLayerVisible && _activeDevice.isSome());
}

void DeviceLayer::setCurrentActive(bool editTemplate)
{
    Layer* layer;
    if (editTemplate) {
        layer = new TemplateLayer(mapRect());
    } else {
        auto activeDevice = this->activeDevice();

        if (activeDevice == nullptr)
            return;

        activeDevice->aircraft()->setActive(true);
        core::FlyingDevice* d = activeDevice->device();
        if (!d->selectedRoute()) {
            return;
        }
        WaypointLayer* wlayer
            = new WaypointLayer(d, d->selectedRoute()->buffer(), mapRect());
        wlayer->setStyle(WaypointLayer::Editable);
        layer = wlayer;
    }
    setCurrentActive(layer);
}

void DeviceLayer::setCurrentDevice(FlyingDevice* device)
{
    if (_activeDevice.isSome()) {
        disconnect(device, &FlyingDevice::routeSelectionChanged, this, 0);
    }
    auto it = std::find_if(
        _layers.begin(), _layers.end(),
        [this, device](const std::unique_ptr<AircraftWaypointsPair>& pair) { return pair->device() == device; });
    if (it != _layers.end()) {
        _activeDevice = it - _layers.begin();
    }
    if (_activeDevice.isSome()) {
        connect(device, &FlyingDevice::routeSelectionChanged, this, [this](core::Route* route) {
            setCurrentActive();
        });
        connect(device, &FlyingDevice::positionChanged, this, &DeviceLayer::currentPositionChanged);
    }
}

void DeviceLayer::reorderLayers()
{
    if (_activeDevice.isSome() && _layers.size() > 1) {
        std::swap(_layers[_activeDevice.unwrap()], *(_layers.end() - 1));
        _activeDevice = _layers.size() - 1;
    }
}

template <typename F, typename... A>
void DeviceLayer::visitLayers(F func, A&&... args)
{
    for (auto& pair : _layers) {
        (pair->aircraft().get()->*func)(std::forward<A>(args)...);
        pair->mapWaypointLayers(func, std::forward<A>(args)...);
    }
    if (_activeDevice.isSome() && _editableWaypointLayer) {
        (_editableWaypointLayer.get()->*func)(std::forward<A>(args)...);
    }
}

template <typename F, typename... A>
inline bool DeviceLayer::doIfVisible(F func, A&&... args)
{
    if (_activeDevice.isSome() && _editableWaypointLayer && _isEditableLayerVisible) {
        return (_editableWaypointLayer.get()->*func)(std::forward<A>(args)...);
    }
    return false;
}

bool DeviceLayer::zoomEvent(const QPoint& pos, int fromZoom, int toZoom)
{
    visitLayers(&Layer::zoomEvent, pos, fromZoom, toZoom);
    return true;
}

void DeviceLayer::changeProjection(const MercatorProjection& from, const MercatorProjection& to)
{
    visitLayers(&Layer::changeProjection, from, to);
}

bool DeviceLayer::contextMenuEvent(const QPoint& pos)
{
    if (!doIfVisible(&Layer::contextMenuEvent, pos)) {
        _menu->exec(mapRect()->toWindowSystemCoordinates(pos));
        return true;
    }
    return false;
}

bool DeviceLayer::mouseMoveEvent(const QPoint& oldPos, const QPoint& newPos)
{
    if (!doIfVisible(&Layer::mouseMoveEvent, oldPos, newPos)) {
        for (std::unique_ptr<AircraftWaypointsPair>& pair : _layers) {
            if (pair->aircraft()->mouseMoveEvent(oldPos, newPos)) {
                return true;
            }
        }
    }
    return false;
}

bool DeviceLayer::trySelectDeviceAt(const QPoint& pos)
{
    for (auto& pair : _layers) {
        if (pair->aircraft()->hasDeviceAt(pos)) {
            _manager->selectDevice(pair->device());
            return true;
        }
    }
    return false;
}

bool DeviceLayer::mousePressEvent(const QPoint& pos)
{
    if (!doIfVisible(&Layer::mousePressEvent, pos)) {
        return trySelectDeviceAt(pos);
    } else {
        return true;
    }
    return false;
}

bool DeviceLayer::mouseReleaseEvent(const QPoint& pos)
{
    return doIfVisible(&Layer::mouseReleaseEvent, pos);
}

bool DeviceLayer::viewportResetEvent(int oldZoom, int newZoom, const QRect& oldViewpiort, const QRect& newViewport)
{
    visitLayers(&Layer::viewportResetEvent, oldZoom, newZoom, oldViewpiort, newViewport);
    return true;
}

bool DeviceLayer::viewportResizeEvent(const QSize& oldSize, const QSize& newSize)
{
    visitLayers(&Layer::viewportResizeEvent, oldSize, newSize);
    return true;
}

bool DeviceLayer::viewportScrollEvent(const QPoint& oldPos, const QPoint& newPos)
{
    visitLayers(&Layer::viewportScrollEvent, oldPos, newPos);
    return true;
}

void DeviceLayer::draw(QPainter* p) const
{
    for (const auto& pair : _layers) {
        pair->draw(p);
    }
    if (_activeDevice.isSome() && _editableWaypointLayer && _isEditableLayerVisible) {
        _editableWaypointLayer->draw(p);
    }
}

void DeviceLayer::setEditableLayerVisible(bool isVisible)
{
    _isEditableLayerVisible = isVisible;
    _resetRouteAction->setEnabled(_isEditableLayerVisible && _activeDevice.isSome());
    _clearRouteAction->setEnabled(_isEditableLayerVisible && _activeDevice.isSome());
    _applyRouteAction->setEnabled(_isEditableLayerVisible && _activeDevice.isSome());
    _endEditRouteAction->setEnabled(_isEditableLayerVisible && _activeDevice.isSome());
}

misc::Option<core::GeoBox> DeviceLayer::currentBbox() const
{
    core::FlyingDevice* device = _manager->currentDevice();
    if (!device) {
        return misc::None;
    }

    if (!device->selectedRoute()) {
        return misc::None;
    }

    core::Route* route = device->selectedRoute()->buffer();
    if (!route) {
        return misc::None;
    }
    if (route->waypointsCount() == 0) {
        return misc::None;
    }
    return route->computeBoundindBox();
}

misc::Option<core::LatLon> DeviceLayer::currentPosition() const
{
    if (_activeDevice.isSome()) {
        return activeDevice()->device()->position().latLon();
    }
    return misc::None;
}
}
}
}
