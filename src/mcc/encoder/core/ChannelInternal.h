/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#pragma once
#include <QDebug>
#include "mcc/misc/Messenger.h"
#include "mcc/encoder/core/Channel.h"


namespace mcc {
namespace encoder {
namespace core {

    class ChannelInternal : public Channel
    {
    public:
        ChannelInternal(const mcc::misc::NetAddress& address, int msTimeout)
            : Channel(address, msTimeout)
        {
        }

        ~ChannelInternal()
        {
        }

        bool isOpen() const override { return _isOpen; /*_socket.isOpen()*/ }

        QString errorString() const override { return ""; }

        bool connect() override
        {
            _isOpen = true; //_socket.connectToHost(address);
            return true;
        }

        void disconnect() override { _isOpen = false; /*_socket.disconnect();*/ }

        bool send(const void* ptr, std::size_t size) override
        {
            QHostAddress host(address().host());
            auto bytesSent = _socket.writeDatagram((const char*)ptr, size, host, address().port());
            if (bytesSent < 0)
            {
                return false;
            }
            _bytesSent += (std::size_t)bytesSent;
            return true;
        }

        bool receive(QByteArray* packet) override
        {
            if (!_socket.waitForReadyRead(timeoutMs()))
            {
                qDebug() << "device is not responding for " << timeoutMs() << "ms";
                return false;
            }

            packet->resize(_socket.pendingDatagramSize());
            _socket.readDatagram(packet->data(), packet->size());
            if (packet->isEmpty())
            {
                qDebug() << "device sent empty packet";
                return false;
            }
            _bytesReceived += packet->size();
            return true;
        };

    private:
        bool        _isOpen = false;
        mcc::misc::Messenger   _messenger;
    };

}
}
}