/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#pragma once
#include <vector>
#include <memory>
#include <cassert>
#include <QString>
#include <QMetaType>
#include "bmcl/Option.h"
#include "mcc/misc/NetStatistics.h"


namespace mcc {
namespace misc {

enum class NetTransport
{
    Unknown = -1,
    Tcp,
    Udp,
    Com,
};

class NetTcpParams
{
public:
    NetTcpParams(const QString& host, uint16_t remotePort, bool autoReconnect = false)
        : _host(host), _remotePort(remotePort)
    {
    }
    const QString&  host() const { return _host; }
    uint16_t remotePort() const { return _remotePort; }
    QString name() const { return QString("%1:%2").arg(_host).arg(_remotePort); }
    static bmcl::Option<NetTcpParams> deserialize(const QString& dump);
    QString serialize() const;
private:
    uint16_t _remotePort;
    QString _host;
    QString _name;
};

class NetUdpParams
{
public:
    NetUdpParams(const QString& host, bmcl::Option<uint16_t> remotePort, bmcl::Option<uint16_t> localPort)
        : _host(host), _remotePort(remotePort), _localPort(localPort)
    {
    }
    const QString&  host() const { return _host; }
    bmcl::Option<uint16_t> remotePort() const { return _remotePort; }
    bmcl::Option<uint16_t> localPort() const { return _localPort; }
    QString name() const
    {
        if (_remotePort.isSome())
            return QString("%1:r%2").arg(_host).arg(_remotePort.unwrap());
        return QString("%1:l%2").arg(_host).arg(_localPort.unwrap());
    }
    static bmcl::Option<NetUdpParams> deserialize(const QString& dump);
    QString serialize() const;
private:
    QString _host;
    bmcl::Option<uint16_t> _remotePort;
    bmcl::Option<uint16_t> _localPort;
};

class NetComParams
{
public:
    NetComParams(const QString& portName, std::size_t baudRate)
        : _baudRate(baudRate), _portName(portName)
    {
    }
    std::size_t    baudRate() const { return _baudRate; }
    const QString& portName() const { return _portName; }
    const QString& name() const { return _portName; }
    static bmcl::Option<NetComParams> deserialize(const QString& dump);
    QString serialize() const;
private:
    std::size_t _baudRate;
    QString     _portName;
};

class NetChannel
{
public:
    NetChannel() :_transport(NetTransport::Unknown){}
    NetChannel(const QString& protocol, const NetTcpParams& params) : _transport(NetTransport::Tcp), _protocol(protocol), _tcp(params)
    {
        _uniqueName = uniqueName(params.name());
    }
    NetChannel(const QString& protocol, const NetUdpParams& params) : _transport(NetTransport::Udp), _protocol(protocol), _udp(params)
    {
        _uniqueName = uniqueName(params.name());
    }
    NetChannel(const QString& protocol, const NetComParams& params) : _transport(NetTransport::Com), _protocol(protocol), _com(params)
    {
        _uniqueName = uniqueName(params.name());
    }

    const QString& name() const { return _uniqueName; }
    NetTransport   transport()  const { return _transport; }
    const QString& protocol()   const { return _protocol; }
    const bmcl::Option<NetTcpParams>& tcp() const { return _tcp; }
    const bmcl::Option<NetUdpParams>& udp() const { return _udp; }
    const bmcl::Option<NetComParams>& com() const { return _com; }

    QString serialize() const;
    static bmcl::Option<NetChannel> deserialize(const QString& dump);
private:
    QString uniqueName(const QString& name) const;

    NetTransport _transport;
    QString      _protocol;
    QString      _uniqueName;
    mutable QString _serialized;

    bmcl::Option<NetTcpParams> _tcp;
    bmcl::Option<NetUdpParams> _udp;
    bmcl::Option<NetComParams> _com;
};

enum class ConnectionState
{
    connected    = 1,
    disconnected = 2,
    pending      = 3
};

struct NetState
{
    NetState(){}
    NetState(const QString& address, ConnectionState state, const QString& error = QString())
        : _state(state)
        , _address(address)
        , _error(error)
    {
    }

    ConnectionState _state;
    QString         _address;
    QString         _error;
    NetStatistics   _stats;
    std::vector<QString> _devices;
};

typedef std::vector<NetState> NetStateList;

}}

Q_DECLARE_METATYPE(mcc::misc::NetState);
Q_DECLARE_METATYPE(mcc::misc::NetChannel);
Q_DECLARE_METATYPE(mcc::misc::NetStateList);
Q_DECLARE_METATYPE(mcc::misc::ConnectionState);

