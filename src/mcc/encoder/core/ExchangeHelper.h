/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#pragma once
#include <memory>
#include <QDebug>

#include "bmcl/MemWriter.h"
#include "bmcl/MemReader.h"
#include "bmcl/Result.h"

#include "mcc/misc/Channel.h"
#include "mcc/encoder/core/Channel.h"
#include "mcc/encoder/core/ExchangeError.h"
#include "mcc/encoder/core/Packet.h"


namespace mcc {
namespace encoder {
namespace core {

#define execute_or_exit(response, expression)           \
    auto temp_##response = expression;                  \
    if (temp_##response.isErr())                        \
        return std::string(toString(temp_##response.takeErr()));     \
    auto response = temp_##response.take();

    template<typename C>
    class ExchangeHelper
    {
    private:
        const std::size_t _tries;
        mcc::misc::ChannelPair<std::vector<uint8_t>> _channel;
    public:
        ExchangeHelper(const mcc::misc::ChannelPair<std::vector<uint8_t>>& channel, std::size_t tries = std::size_t(-1)) : _tries(tries), _channel(channel)
        {
            assert(tries > 0);
        }

        //Отправка/Приём нетипизированного пакета из ChannelPair. Пакет должен быть передан за один "подход"
        bmcl::Result<std::vector<uint8_t>, ExchangeError>  requestReply(const std::vector<uint8_t>& in)
        {
            auto tries = _tries;
            ExchangeError err;
            while (tries > 0)
            {
                --tries;
                if (!_channel.sender.send(in))
                    return ExchangeError::PacketCantSend;

                auto opt = _channel.reciever.recv();
                if (opt.isNone())
                    return ExchangeError::PacketCantReceive;

                auto pktOut = opt.take();
                auto r = validatePacket<C>(pktOut.data(), pktOut.size());
                if (r.isNone())
                    return std::move(pktOut);

                err = r.take();
                if (isResendImpossible(err))
                    return err;
            }

            return err;
        }

        template<typename T>
        bmcl::Result<typename T::Response, ExchangeError> requestReply(const typename T::Request& request, std::size_t tries)
        {
            auto p = C::encodePacket(&request);
            if (p.isErr())
                return p.takeErr();

            auto packet = p.take();
            ExchangeError err;
            while (tries)
            {
                --tries;
                if (!_channel.sender.send(packet))
                    return ExchangeError::PacketCantSend;

                auto opt = _channel.reciever.recv();
                if (opt.isNone())
                    return ExchangeError::PacketCantReceive;

                auto out = opt.take();
                auto r = C::template decode<typename T::Response>(out.data(), out.size());
                if (r.isOk())
                    return r.take();
                err = r.takeErr();
                if (isResendImpossible(err))
                    return err;
            }

            return err;
        };

        template<typename T>
        bmcl::Result<typename T::Response, ExchangeError> requestReply(const typename T::Request& request)
        {
            return requestReply<T>(request, _tries);
        }

    };

}
}
}
