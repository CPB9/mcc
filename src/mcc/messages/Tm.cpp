/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "mcc/messages/Tm.h"
#include "mcc/messages/protobuf/Message.pb.h"


namespace mcc {
namespace messages {

MESSAGE_REQUEREMENT_DEFINITIONS(TmParamSubscribe_Request);
MESSAGE_REQUEREMENT_DEFINITIONS(TmParamSubscribe_Response);
MESSAGE_REQUEREMENT_DEFINITIONS(TmParamList);

void TmParamList::serialize_(mcc::protobuf::MessageBody* body) const
{
    auto tm = body->mutable__tmparamlist();
    tm->set_device(_device);
    auto params = tm->mutable_params();
    params->Reserve(_params.size());
    for (const auto& i : _params)
    {
        auto p = params->Add();
        p->set_trait(i.trait());
        p->set_status(i.status());
        p->set_value(i.value().serialize());
    }
}

std::unique_ptr<Message> TmParamList::deserialize(const mcc::protobuf::TmParamList& list)
{
    TmParams params;
    params.reserve(list.params().size());
    for (const auto& i : list.params())
    {
        bmcl::MemReader reader(i.value().data(), i.value().size());
        auto p = mcc::misc::NetVariant::deserialize(&reader);
        if (p.isErr())
            return nullptr;
        params.emplace_back(i.trait(), i.status(), p.take());
    }

    return mcc::misc::makeUnique<TmParamList>(list.device(), std::move(params));
}

}
}

