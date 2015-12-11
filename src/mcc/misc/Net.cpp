/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include <QStringList>
#include "mcc/misc/Net.h"


namespace mcc {
namespace misc {

inline const char* toString(NetTransport transport)
{
    switch (transport)
    {
    case NetTransport::Unknown: return "unknown";
    case NetTransport::Tcp:     return "tcp";
    case NetTransport::Udp:     return "udp";
    case NetTransport::Com:     return "com";
    }
    return "unknown";
}

inline bmcl::Option<NetTransport> fromString(const QString& transport)
{
    if (transport == "tcp")     return NetTransport::Tcp;
    if (transport == "udp")     return NetTransport::Udp;
    if (transport == "com")     return NetTransport::Com;
    return bmcl::None;
}

QString NetChannel::uniqueName(const QString& name) const
{
    return QString("%1:%2").arg(toString(_transport)).arg(name);
}

QString NetChannel::serialize() const
{
    if (!_serialized.isEmpty())
        return _serialized;

    QString r;
    switch (_transport)
    {
    case NetTransport::Tcp: r = _tcp->serialize(); break;
    case NetTransport::Udp: r = _udp->serialize(); break;
    case NetTransport::Com: r = _com->serialize(); break;
    default:
        assert(false);
    }
    _serialized = QString("%1:%2:%3").arg(_protocol).arg(toString(_transport)).arg(r);
    return _serialized;
}

bmcl::Option<NetChannel> NetChannel::deserialize(const QString& dump)
{
    QStringList split = dump.split(":");
    if (split.size() < 3)
        return bmcl::None;

    auto t = fromString(split[1]);
    if (t.isNone())
        return bmcl::None;

    switch (t.unwrap())
    {
    case NetTransport::Tcp:
        {
            auto r = NetTcpParams::deserialize(dump.section(":", 2));
            if (r.isSome())
                return NetChannel(split[0], r.take());
        }
        break;
    case NetTransport::Udp:
        {
            auto r = NetUdpParams::deserialize(dump.section(":", 2));
            if (r.isSome())
                return NetChannel(split[0], r.take());
        }
        break;
    case NetTransport::Com:
        {
            auto r = NetComParams::deserialize(dump.section(":", 2));
            if (r.isSome())
                return NetChannel(split[0], r.take());
        }
        break;
    default:
        break;
    }
    assert(false);
    return bmcl::None;
}

QString NetTcpParams::serialize() const
{
    return QString("%1:%2").arg(_host).arg(_remotePort);
}

bmcl::Option<NetTcpParams> NetTcpParams::deserialize(const QString& dump)
{
    auto split = dump.split(":");
    if (split.size() != 2)
    {
        assert(false);
        return bmcl::None;
    }
    return NetTcpParams(split[0], split[1].toUInt());
}

QString NetUdpParams::serialize() const
{
    QString r = _remotePort.isSome() ? QString::number(_remotePort.unwrap()) : QString();
    QString l = _localPort.isSome() ? QString::number(_localPort.unwrap()) : QString();
    return QString("%1:%2:%3").arg(_host).arg(r).arg(l);
}

bmcl::Option<NetUdpParams> NetUdpParams::deserialize(const QString& dump)
{
    auto split = dump.split(":");
    if (split.size() != 3)
    {
        assert(false);
        return bmcl::None;
    }
    bmcl::Option<uint16_t> r;
    if (!split[1].isEmpty())
        r = split[1].toUInt();

    bmcl::Option<uint16_t> l;
    if (!split[2].isEmpty())
        l = split[2].toUInt();

    return NetUdpParams(split[0], r, l);
}

QString NetComParams::serialize() const
{
    return QString("%1:%2").arg(_portName).arg(_baudRate);
}

bmcl::Option<NetComParams> NetComParams::deserialize(const QString& dump)
{
    auto split = dump.split(":");
    if (split.size() != 2)
    {
        assert(false);
        return bmcl::None;
    }
    return NetComParams(split[0], split[1].toUInt());
}


}}

