/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "mcc/encoder/core/Channel.h"
#include "mcc/encoder/core/ChannelCom.h"
#include "mcc/encoder/core/ChannelUdp.h"
#include "mcc/encoder/core/ChannelTcp.h"


namespace mcc {
namespace encoder {
namespace core {

template<typename C, typename A>
std::unique_ptr<Channel> createChannel(const bmcl::Option<A>& params)
{
    return std::unique_ptr<Channel>((Channel*)(new C(params.unwrap())));
}

std::unique_ptr<Channel> Channel::makeChannel(const std::string& settings)
{
    auto r = mcc::misc::NetChannel::deserialize(QString::fromStdString(settings));
    if (r.isNone())
        return nullptr;

    switch (r->transport())
    {
    case mcc::misc::NetTransport::Com: return createChannel<ChannelCom, mcc::misc::NetComParams>(r->com());
    case mcc::misc::NetTransport::Udp: return createChannel<ChannelUdp, mcc::misc::NetUdpParams>(r->udp());
    case mcc::misc::NetTransport::Tcp: return createChannel<ChannelTcp, mcc::misc::NetTcpParams>(r->tcp());
    }
    return nullptr;
}
}
}
}
