/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#pragma once

#include <cstddef>
#include <cstdint>

namespace mcc {
namespace misc {

uint16_t crc16Modbus(const void* data, std::size_t len);

uint16_t crc16X25(const void* data, std::size_t len);

uint16_t crc16Mcrf4xx(const void* data, std::size_t len);

uint32_t crc32(const void* data, std::size_t len);

}
}
