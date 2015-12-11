/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#pragma once
#include <QMetaType>
#include <QString>
#include "mcc/misc/NetStatistics.h"

namespace mcc {
namespace misc {

struct DeviceDescription
{
    DeviceDescription(){}
    DeviceDescription(const QString& device_name) : _device_name(device_name){}
    std::size_t _device_id = std::size_t(-1);
    std::size_t _firmware_id = std::size_t(-1);
    std::size_t _kind_id = std::size_t(-1);
    QString     _device_name;
    QString     _device_info;
    QString     _kind_name;
    QString     _kind_info;
    QString     _firmware_name;
    QString     _firmware_info;
};

struct FileTransfer
{
    enum class Direction
    {
        Up,
        Down
    };

    FileTransfer(){}
    FileTransfer(const std::string& file_path, std::size_t fileSizeBytes, std::size_t uploadedBytes, Direction direction)
        : _direction(direction), _file_path(file_path), _fileSizeBytes(fileSizeBytes), _uploadedBytes(uploadedBytes){}
    inline double progress() const
    {
        if (_fileSizeBytes == 0)
            return 0;
        return 100.0 * (double)_uploadedBytes / (double)_fileSizeBytes;
    }
    Direction   _direction;
    std::string _file_path;
    std::size_t _fileSizeBytes;
    std::size_t _uploadedBytes;
};
typedef std::vector<FileTransfer> FileTransfers;

struct DeviceState
{
    DeviceState(){}
    DeviceState(const QString& device, bool isActive, bool isRegistered, std::size_t regState)
        : _isActive(isActive)
        , _isRegistered(isRegistered)
        , _regState(regState)
        , _device(device)
    {
    }

    bool            _isActive = false;
    bool            _isRegistered = false;
    std::size_t     _regState = 0;
    QString         _device;
    NetStatistics   _stats;
    FileTransfers   _files;
};

typedef std::vector<DeviceState> DeviceStateList;
}
}

Q_DECLARE_METATYPE(mcc::misc::DeviceDescription);
Q_DECLARE_METATYPE(mcc::misc::DeviceState);
Q_DECLARE_METATYPE(mcc::misc::DeviceStateList);
