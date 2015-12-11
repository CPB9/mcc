/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#pragma once

#include "mcc/misc/Route.h"
#include "mcc/modeling/2DMath.h"
#include "mcc/messages/LocalRouter.h"
#include "mcc/messages/ServiceAbstract.h"

using mcc::misc::PointCoordinates;

namespace mcc
{
namespace modeling
{

enum class SimplifiedRouteKind : uint16_t
{
    Manual = 0, Auto
};

enum class RouteKind : uint16_t
{
    Manual = 0, Buffer, Auto
};

inline QDebug &operator<<(QDebug &debug, RouteKind &routeKind)
{
    switch (routeKind)
    {
    case RouteKind::Manual:
        return debug << "RouteKind::Manual";
    case RouteKind::Buffer:
        return debug << "RouteKind::Buffer";
    case RouteKind::Auto:
        return debug << "RouteKind::Auto";
    }
    assert(false);
}

class RoutePoint : public PointCoordinates
{
public:
    RoutePoint(double latitudeDeg, double longitudeDeg, double altitudeDeg, double speedMps = 0., uint32_t flags = 0)
            : RoutePoint(PointCoordinates(latitudeDeg, longitudeDeg, altitudeDeg), speedMps, flags) {}
    RoutePoint(PointCoordinates coordinates = {0., 0., 0.}, double speedMps = 0., uint32_t flags = 0)
    : PointCoordinates(coordinates)
    , _speedMps(speedMps)
    , _flags(flags)
    {
    }
    double speedMps() const { return _speedMps; }
    uint32_t flags() const { return _flags; }

private:
    double _speedMps;
    uint32_t _flags;
};

inline QDebug &operator<<(QDebug &debug, const RoutePoint &routePoint)
{
    return debug << "RoutePoint(" << routePoint.longitudeDeg() << routePoint.latitudeDeg() << routePoint.altitude()
           << routePoint.speedMps() << ")";
}

class Route
{
public:
    explicit Route(uint16_t name = 0, SimplifiedRouteKind kind = SimplifiedRouteKind::Auto,
        bool isRing = false, uint16_t crc16 = 0, uint16_t maxLength = 50, std::string info = std::string(),
        const std::vector<RoutePoint> & points = {})
            : _name(name)
            , _kind(kind)
            , _isRing(isRing)
            , _crc16(crc16)
            , _maxLength(maxLength)
            , _info(info)
            , _points(points)
    {
    }
    Route(const Route &route) = default;
    Route &operator=(const Route &route) = default;
    Route(Route &&route)
    {
        _name = route._name;
        _kind = route._kind;
        _isRing = route._isRing;
        _crc16 = route._crc16;
        _maxLength = route._maxLength;
        _info = route._info;
        _points = std::move(route._points);
    }
    uint16_t name() const { return _name; }
    void setName(uint16_t name) { _name = name; }
    SimplifiedRouteKind kind() const { return _kind; }
    uint16_t crc16() const { return _crc16; }
    uint16_t maxLength() const { return _maxLength; }
    const std::string& info() const { return _info; }
    void clear() { _points.clear(); }
    void setIsRing(bool isRing) { _isRing = isRing; }
    void setCrc16(uint16_t crc16) { _crc16 = crc16; }
    template<typename... K>
    void appendPoint(K&&... args)
    {
        _points.emplace_back(std::forward<K>(args)...);
    }
    bool isRing() const { return _isRing; }
    const RoutePoint &point(size_t index) const
    {
        BMCL_ASSERT_MSG(index < _points.size(), "invalid point index");
        return _points.at(index);
    }
    size_t length() const { return _points.size(); }
    const std::vector<RoutePoint> & points() const { return _points; }

private:
    uint16_t _name;
    SimplifiedRouteKind _kind;
    bool _isRing;
    uint16_t _crc16, _maxLength;
    std::string _info;
    std::vector<RoutePoint> _points;
};

class Model : public mcc::messages::ServiceAbstract
{
public:
    Model(const mcc::messages::LocalRouterPtr& router, const std::string& serviceName, const std::string& deviceName, double tickS = 1.0)
        : ServiceAbstract(serviceName, router), _deviceName(deviceName), _tickS(tickS)
    {
    }
    virtual ~Model() {}
    void setTickS(double tickS) { _tickS = tickS; }
    const std::string& deviceName() const { return _deviceName; }
    virtual void step() = 0;
protected:

    std::string _deviceName;
    double _tickS;
};

}
}
