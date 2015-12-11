/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#pragma once
#include "mcc/encoder/core/Service.h"


namespace mcc {
namespace encoder {
namespace mavlink {


class Service : public mcc::encoder::core::Service
{
public:
    Service(const mcc::messages::LocalRouterPtr& router);
    virtual ~Service();
protected:
    std::shared_ptr<mcc::encoder::core::Device> createDevice(const mcc::misc::ProtocolId& id, const std::string& device_name) override;
};
}
}
}
