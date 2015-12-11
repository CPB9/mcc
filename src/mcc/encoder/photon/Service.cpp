/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include <QDebug>
#include <QCoreApplication>
#include "bmcl/Result.h"

#include "mcc/Names.h"
#include "mcc/messages/Deaclarations.h"
#include "mcc/misc/Crc.h"
#include "mcc/encoder/core/ChannelManager.h"
#include "mcc/encoder/photon/Service.h"
#include "mcc/encoder/photon/Device.h"
#include "mcc/encoder/photon/Photon.h"


namespace mcc {
namespace encoder {
namespace photon {

Service::Service(const mcc::messages::LocalRouterPtr& router)
    : mcc::encoder::core::Service(mcc::Names::encoderPhoton(), router, "Photon", mcc::encoder::core::PacketSearcher(&mcc::encoder::core::findPacket<PhotonCoder>), false)
{
}

Service::~Service()
{
    finish(true);
}

std::shared_ptr<mcc::encoder::core::Device> Service::createDevice(const mcc::misc::ProtocolId& id, const std::string& device_name)
{
    return std::make_shared<Device>(id, _out, device_name);
}

}
}
}
