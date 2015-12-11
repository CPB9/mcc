/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#pragma once
#include <string>
#include <vector>
#include "mcc/misc/TimeUtils.h"


namespace mcc {
namespace messages {

struct Stat
{
    void reset()
    {
        _packets = 0;
        _bytes = 0;
        _time = mcc::misc::currentDateTime();
    }
    void add(std::size_t bytes, bool isPacket = true)
    {
        _bytes += bytes;
        if (isPacket) _packets += 1;
        _time = mcc::misc::currentDateTime();
    }
    std::size_t _packets;
    std::size_t _bytes;
    std::string _time;
};

struct StatChannel
{
    void reset()
    {
        _sent.reset();
        _rcvd.reset();
        _bad.reset();
    }
    bool _isConnected;
    Stat _sent;
    Stat _rcvd;
    Stat _bad;
    std::string _address;
    std::vector<std::string> _devices;
};
typedef std::vector<StatChannel> StatChannels;

struct StatFile
{
    bool _isUp;
    std::size_t _fileSize;
    std::size_t _bytesLoaded;
    std::string _file_path;
};
typedef std::vector<StatFile> StatFiles;

struct StatDevice
{
    void reset()
    {
        _sent.reset();
        _rcvd.reset();
        _bad.reset();
        _cmdsInQueue = 0;
        _cmdsProcessed = 0;
        _regState = 100;
        _files.clear();
        _isActive = false;
        _isRegistered = false;
    }
    bool _isActive;
    bool _isRegistered;
    std::size_t _regState = 100;
    std::size_t _cmdsInQueue;
    std::size_t _cmdsProcessed;

    Stat _sent;
    Stat _rcvd;
    Stat _bad;
    std::string _device;
    StatFiles _files;
};
typedef std::vector<StatDevice> StatDevices;

}
}

