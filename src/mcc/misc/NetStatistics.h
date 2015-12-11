/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#pragma once
#include <cstdint>
#include <QDateTime>


namespace mcc {
namespace misc {

struct NetStatistics
{
    void sent(std::size_t bytes)
    {
        _sentBytes += bytes;
        ++_sentPackets;
        _sent = QDateTime::currentDateTime();
    }
    void rcvd(std::size_t bytes, bool isGood)
    {
        if (isGood)
        {
            _rcvdBytes += bytes;
            ++_rcvdPackets;
            _rcvd = QDateTime::currentDateTime();
        }
        else
        {
            _rcvdBadBytes += bytes;
            ++_rcvdBadPackets;
            _rcvdBad = QDateTime::currentDateTime();
        }
    }

    void reset()
    {
        _sentBytes = 0;
        _sentPackets = 0;
        _rcvdBytes = 0;
        _rcvdPackets = 0;
        _sent = QDateTime::currentDateTime();
        _rcvd = QDateTime::currentDateTime();
        _rcvdBad = QDateTime::currentDateTime();
    }
    std::size_t _sentBytes    = 0;
    std::size_t _sentPackets  = 0;
    std::size_t _rcvdBytes   = 0;
    std::size_t _rcvdPackets = 0;
    std::size_t _rcvdBadBytes = 0;
    std::size_t _rcvdBadPackets = 0;
    QDateTime   _sent;
    QDateTime   _rcvd;
    QDateTime   _rcvdBad;
};

}
}