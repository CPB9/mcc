/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#pragma once
#include <cstdint>
#include <memory>
#include <thread>
#include <vector>
#include <future>
#include <string>

#include "mcc/misc/Channel.h"
#include "mcc/misc/Net.h"
#include "mcc/messages/Deaclarations.h"
#include "mcc/misc/NetStatistics.h"
#include "mcc/misc/Runnable.h"

#include "mcc/encoder/core/Channel.h"
#include "mcc/encoder/core/Device.h"


namespace mcc {
namespace encoder {
namespace core {

class ChannelManager : public mcc::misc::Runnable
{
public:
    ChannelManager(bool syncExchange
        , std::unique_ptr<mcc::messages::Channel_Request>&& request
        , const mcc::messages::MessageSender& out
        , const mcc::encoder::core::PacketSearcher& searcher
        , const ChannelOpenerPtr& channelOpener
        , std::size_t syncResponseTimeoutMs = 100
        , std::size_t syncNoExchangePauseMs = 1
        , std::size_t asyncSleepOnReceiveMs = 100
        );

    virtual ~ChannelManager();
    const std::string& address() const { return _exchanger.address(); }

    void request(std::unique_ptr<mcc::messages::Channel_Request>&& request);
    bool isOpen() const;

    void addDevice(const std::shared_ptr<Device>& device);
    void removeDevice(const std::string& name);

    mcc::messages::StatChannel getStatistics() const;

protected:
    bool pre();
    void tick();
    void post();

private:
    void executeRequest_();
    bool isSleepCondition_(const std::vector<std::shared_ptr<Device>>& devices) const;
    void syncExchange_(const std::vector<std::shared_ptr<Device>>& devices);
    void asyncExchange_(const std::vector<std::shared_ptr<Device>>& devices);

    std::size_t _syncResponseTimeoutMs;
    std::size_t _syncNoExchangePauseMs;
    std::size_t _asyncSleepOnReceiveMs;


    bool _syncExchange;
    std::unique_ptr<mcc::messages::Channel_Request> _request;
    std::atomic<bool>           _hasRequest;

    bool _isDevicesChanged;
    std::vector<std::shared_ptr<Device>> _devicesTemp;
    std::vector<std::shared_ptr<Device>> _devices;

    mcc::messages::StatChannel _stats;
    mcc::messages::MessageSender _out;
    mcc::encoder::core::Exchanger _exchanger;

    std::vector<uint8_t> _bufferIn;
    std::vector<uint8_t> _bufferOut;
};

}
}
}