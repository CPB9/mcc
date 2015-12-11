/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

// NOTE: работает только на little-endian

#include "mcc/misc/Crc.h"

namespace mcc {
namespace misc {

/*
 * CRC16-MODBUS
 *
 * Code generated by universal_crc by Danjel McGougan
 *
 * CRC parameters used:
 *   bits:       16
 *   poly:       0x8005
 *   init:       0xffff
 *   xor:        0x0000
 *   reverse:    true
 *   non-direct: false
 *
 * CRC of the string "123456789" is 0x4b37
 */

const uint16_t crc16ModbusTable[1024]
    = {0x0000, 0xc0c1, 0xc181, 0x0140, 0xc301, 0x03c0, 0x0280, 0xc241, 0xc601, 0x06c0, 0x0780, 0xc741, 0x0500, 0xc5c1,
       0xc481, 0x0440, 0xcc01, 0x0cc0, 0x0d80, 0xcd41, 0x0f00, 0xcfc1, 0xce81, 0x0e40, 0x0a00, 0xcac1, 0xcb81, 0x0b40,
       0xc901, 0x09c0, 0x0880, 0xc841, 0xd801, 0x18c0, 0x1980, 0xd941, 0x1b00, 0xdbc1, 0xda81, 0x1a40, 0x1e00, 0xdec1,
       0xdf81, 0x1f40, 0xdd01, 0x1dc0, 0x1c80, 0xdc41, 0x1400, 0xd4c1, 0xd581, 0x1540, 0xd701, 0x17c0, 0x1680, 0xd641,
       0xd201, 0x12c0, 0x1380, 0xd341, 0x1100, 0xd1c1, 0xd081, 0x1040, 0xf001, 0x30c0, 0x3180, 0xf141, 0x3300, 0xf3c1,
       0xf281, 0x3240, 0x3600, 0xf6c1, 0xf781, 0x3740, 0xf501, 0x35c0, 0x3480, 0xf441, 0x3c00, 0xfcc1, 0xfd81, 0x3d40,
       0xff01, 0x3fc0, 0x3e80, 0xfe41, 0xfa01, 0x3ac0, 0x3b80, 0xfb41, 0x3900, 0xf9c1, 0xf881, 0x3840, 0x2800, 0xe8c1,
       0xe981, 0x2940, 0xeb01, 0x2bc0, 0x2a80, 0xea41, 0xee01, 0x2ec0, 0x2f80, 0xef41, 0x2d00, 0xedc1, 0xec81, 0x2c40,
       0xe401, 0x24c0, 0x2580, 0xe541, 0x2700, 0xe7c1, 0xe681, 0x2640, 0x2200, 0xe2c1, 0xe381, 0x2340, 0xe101, 0x21c0,
       0x2080, 0xe041, 0xa001, 0x60c0, 0x6180, 0xa141, 0x6300, 0xa3c1, 0xa281, 0x6240, 0x6600, 0xa6c1, 0xa781, 0x6740,
       0xa501, 0x65c0, 0x6480, 0xa441, 0x6c00, 0xacc1, 0xad81, 0x6d40, 0xaf01, 0x6fc0, 0x6e80, 0xae41, 0xaa01, 0x6ac0,
       0x6b80, 0xab41, 0x6900, 0xa9c1, 0xa881, 0x6840, 0x7800, 0xb8c1, 0xb981, 0x7940, 0xbb01, 0x7bc0, 0x7a80, 0xba41,
       0xbe01, 0x7ec0, 0x7f80, 0xbf41, 0x7d00, 0xbdc1, 0xbc81, 0x7c40, 0xb401, 0x74c0, 0x7580, 0xb541, 0x7700, 0xb7c1,
       0xb681, 0x7640, 0x7200, 0xb2c1, 0xb381, 0x7340, 0xb101, 0x71c0, 0x7080, 0xb041, 0x5000, 0x90c1, 0x9181, 0x5140,
       0x9301, 0x53c0, 0x5280, 0x9241, 0x9601, 0x56c0, 0x5780, 0x9741, 0x5500, 0x95c1, 0x9481, 0x5440, 0x9c01, 0x5cc0,
       0x5d80, 0x9d41, 0x5f00, 0x9fc1, 0x9e81, 0x5e40, 0x5a00, 0x9ac1, 0x9b81, 0x5b40, 0x9901, 0x59c0, 0x5880, 0x9841,
       0x8801, 0x48c0, 0x4980, 0x8941, 0x4b00, 0x8bc1, 0x8a81, 0x4a40, 0x4e00, 0x8ec1, 0x8f81, 0x4f40, 0x8d01, 0x4dc0,
       0x4c80, 0x8c41, 0x4400, 0x84c1, 0x8581, 0x4540, 0x8701, 0x47c0, 0x4680, 0x8641, 0x8201, 0x42c0, 0x4380, 0x8341,
       0x4100, 0x81c1, 0x8081, 0x4040, 0x0000, 0x9001, 0x6001, 0xf000, 0xc002, 0x5003, 0xa003, 0x3002, 0xc007, 0x5006,
       0xa006, 0x3007, 0x0005, 0x9004, 0x6004, 0xf005, 0xc00d, 0x500c, 0xa00c, 0x300d, 0x000f, 0x900e, 0x600e, 0xf00f,
       0x000a, 0x900b, 0x600b, 0xf00a, 0xc008, 0x5009, 0xa009, 0x3008, 0xc019, 0x5018, 0xa018, 0x3019, 0x001b, 0x901a,
       0x601a, 0xf01b, 0x001e, 0x901f, 0x601f, 0xf01e, 0xc01c, 0x501d, 0xa01d, 0x301c, 0x0014, 0x9015, 0x6015, 0xf014,
       0xc016, 0x5017, 0xa017, 0x3016, 0xc013, 0x5012, 0xa012, 0x3013, 0x0011, 0x9010, 0x6010, 0xf011, 0xc031, 0x5030,
       0xa030, 0x3031, 0x0033, 0x9032, 0x6032, 0xf033, 0x0036, 0x9037, 0x6037, 0xf036, 0xc034, 0x5035, 0xa035, 0x3034,
       0x003c, 0x903d, 0x603d, 0xf03c, 0xc03e, 0x503f, 0xa03f, 0x303e, 0xc03b, 0x503a, 0xa03a, 0x303b, 0x0039, 0x9038,
       0x6038, 0xf039, 0x0028, 0x9029, 0x6029, 0xf028, 0xc02a, 0x502b, 0xa02b, 0x302a, 0xc02f, 0x502e, 0xa02e, 0x302f,
       0x002d, 0x902c, 0x602c, 0xf02d, 0xc025, 0x5024, 0xa024, 0x3025, 0x0027, 0x9026, 0x6026, 0xf027, 0x0022, 0x9023,
       0x6023, 0xf022, 0xc020, 0x5021, 0xa021, 0x3020, 0xc061, 0x5060, 0xa060, 0x3061, 0x0063, 0x9062, 0x6062, 0xf063,
       0x0066, 0x9067, 0x6067, 0xf066, 0xc064, 0x5065, 0xa065, 0x3064, 0x006c, 0x906d, 0x606d, 0xf06c, 0xc06e, 0x506f,
       0xa06f, 0x306e, 0xc06b, 0x506a, 0xa06a, 0x306b, 0x0069, 0x9068, 0x6068, 0xf069, 0x0078, 0x9079, 0x6079, 0xf078,
       0xc07a, 0x507b, 0xa07b, 0x307a, 0xc07f, 0x507e, 0xa07e, 0x307f, 0x007d, 0x907c, 0x607c, 0xf07d, 0xc075, 0x5074,
       0xa074, 0x3075, 0x0077, 0x9076, 0x6076, 0xf077, 0x0072, 0x9073, 0x6073, 0xf072, 0xc070, 0x5071, 0xa071, 0x3070,
       0x0050, 0x9051, 0x6051, 0xf050, 0xc052, 0x5053, 0xa053, 0x3052, 0xc057, 0x5056, 0xa056, 0x3057, 0x0055, 0x9054,
       0x6054, 0xf055, 0xc05d, 0x505c, 0xa05c, 0x305d, 0x005f, 0x905e, 0x605e, 0xf05f, 0x005a, 0x905b, 0x605b, 0xf05a,
       0xc058, 0x5059, 0xa059, 0x3058, 0xc049, 0x5048, 0xa048, 0x3049, 0x004b, 0x904a, 0x604a, 0xf04b, 0x004e, 0x904f,
       0x604f, 0xf04e, 0xc04c, 0x504d, 0xa04d, 0x304c, 0x0044, 0x9045, 0x6045, 0xf044, 0xc046, 0x5047, 0xa047, 0x3046,
       0xc043, 0x5042, 0xa042, 0x3043, 0x0041, 0x9040, 0x6040, 0xf041, 0x0000, 0xc051, 0xc0a1, 0x00f0, 0xc141, 0x0110,
       0x01e0, 0xc1b1, 0xc281, 0x02d0, 0x0220, 0xc271, 0x03c0, 0xc391, 0xc361, 0x0330, 0xc501, 0x0550, 0x05a0, 0xc5f1,
       0x0440, 0xc411, 0xc4e1, 0x04b0, 0x0780, 0xc7d1, 0xc721, 0x0770, 0xc6c1, 0x0690, 0x0660, 0xc631, 0xca01, 0x0a50,
       0x0aa0, 0xcaf1, 0x0b40, 0xcb11, 0xcbe1, 0x0bb0, 0x0880, 0xc8d1, 0xc821, 0x0870, 0xc9c1, 0x0990, 0x0960, 0xc931,
       0x0f00, 0xcf51, 0xcfa1, 0x0ff0, 0xce41, 0x0e10, 0x0ee0, 0xceb1, 0xcd81, 0x0dd0, 0x0d20, 0xcd71, 0x0cc0, 0xcc91,
       0xcc61, 0x0c30, 0xd401, 0x1450, 0x14a0, 0xd4f1, 0x1540, 0xd511, 0xd5e1, 0x15b0, 0x1680, 0xd6d1, 0xd621, 0x1670,
       0xd7c1, 0x1790, 0x1760, 0xd731, 0x1100, 0xd151, 0xd1a1, 0x11f0, 0xd041, 0x1010, 0x10e0, 0xd0b1, 0xd381, 0x13d0,
       0x1320, 0xd371, 0x12c0, 0xd291, 0xd261, 0x1230, 0x1e00, 0xde51, 0xdea1, 0x1ef0, 0xdf41, 0x1f10, 0x1fe0, 0xdfb1,
       0xdc81, 0x1cd0, 0x1c20, 0xdc71, 0x1dc0, 0xdd91, 0xdd61, 0x1d30, 0xdb01, 0x1b50, 0x1ba0, 0xdbf1, 0x1a40, 0xda11,
       0xdae1, 0x1ab0, 0x1980, 0xd9d1, 0xd921, 0x1970, 0xd8c1, 0x1890, 0x1860, 0xd831, 0xe801, 0x2850, 0x28a0, 0xe8f1,
       0x2940, 0xe911, 0xe9e1, 0x29b0, 0x2a80, 0xead1, 0xea21, 0x2a70, 0xebc1, 0x2b90, 0x2b60, 0xeb31, 0x2d00, 0xed51,
       0xeda1, 0x2df0, 0xec41, 0x2c10, 0x2ce0, 0xecb1, 0xef81, 0x2fd0, 0x2f20, 0xef71, 0x2ec0, 0xee91, 0xee61, 0x2e30,
       0x2200, 0xe251, 0xe2a1, 0x22f0, 0xe341, 0x2310, 0x23e0, 0xe3b1, 0xe081, 0x20d0, 0x2020, 0xe071, 0x21c0, 0xe191,
       0xe161, 0x2130, 0xe701, 0x2750, 0x27a0, 0xe7f1, 0x2640, 0xe611, 0xe6e1, 0x26b0, 0x2580, 0xe5d1, 0xe521, 0x2570,
       0xe4c1, 0x2490, 0x2460, 0xe431, 0x3c00, 0xfc51, 0xfca1, 0x3cf0, 0xfd41, 0x3d10, 0x3de0, 0xfdb1, 0xfe81, 0x3ed0,
       0x3e20, 0xfe71, 0x3fc0, 0xff91, 0xff61, 0x3f30, 0xf901, 0x3950, 0x39a0, 0xf9f1, 0x3840, 0xf811, 0xf8e1, 0x38b0,
       0x3b80, 0xfbd1, 0xfb21, 0x3b70, 0xfac1, 0x3a90, 0x3a60, 0xfa31, 0xf601, 0x3650, 0x36a0, 0xf6f1, 0x3740, 0xf711,
       0xf7e1, 0x37b0, 0x3480, 0xf4d1, 0xf421, 0x3470, 0xf5c1, 0x3590, 0x3560, 0xf531, 0x3300, 0xf351, 0xf3a1, 0x33f0,
       0xf241, 0x3210, 0x32e0, 0xf2b1, 0xf181, 0x31d0, 0x3120, 0xf171, 0x30c0, 0xf091, 0xf061, 0x3030, 0x0000, 0xfc01,
       0xb801, 0x4400, 0x3001, 0xcc00, 0x8800, 0x7401, 0x6002, 0x9c03, 0xd803, 0x2402, 0x5003, 0xac02, 0xe802, 0x1403,
       0xc004, 0x3c05, 0x7805, 0x8404, 0xf005, 0x0c04, 0x4804, 0xb405, 0xa006, 0x5c07, 0x1807, 0xe406, 0x9007, 0x6c06,
       0x2806, 0xd407, 0xc00b, 0x3c0a, 0x780a, 0x840b, 0xf00a, 0x0c0b, 0x480b, 0xb40a, 0xa009, 0x5c08, 0x1808, 0xe409,
       0x9008, 0x6c09, 0x2809, 0xd408, 0x000f, 0xfc0e, 0xb80e, 0x440f, 0x300e, 0xcc0f, 0x880f, 0x740e, 0x600d, 0x9c0c,
       0xd80c, 0x240d, 0x500c, 0xac0d, 0xe80d, 0x140c, 0xc015, 0x3c14, 0x7814, 0x8415, 0xf014, 0x0c15, 0x4815, 0xb414,
       0xa017, 0x5c16, 0x1816, 0xe417, 0x9016, 0x6c17, 0x2817, 0xd416, 0x0011, 0xfc10, 0xb810, 0x4411, 0x3010, 0xcc11,
       0x8811, 0x7410, 0x6013, 0x9c12, 0xd812, 0x2413, 0x5012, 0xac13, 0xe813, 0x1412, 0x001e, 0xfc1f, 0xb81f, 0x441e,
       0x301f, 0xcc1e, 0x881e, 0x741f, 0x601c, 0x9c1d, 0xd81d, 0x241c, 0x501d, 0xac1c, 0xe81c, 0x141d, 0xc01a, 0x3c1b,
       0x781b, 0x841a, 0xf01b, 0x0c1a, 0x481a, 0xb41b, 0xa018, 0x5c19, 0x1819, 0xe418, 0x9019, 0x6c18, 0x2818, 0xd419,
       0xc029, 0x3c28, 0x7828, 0x8429, 0xf028, 0x0c29, 0x4829, 0xb428, 0xa02b, 0x5c2a, 0x182a, 0xe42b, 0x902a, 0x6c2b,
       0x282b, 0xd42a, 0x002d, 0xfc2c, 0xb82c, 0x442d, 0x302c, 0xcc2d, 0x882d, 0x742c, 0x602f, 0x9c2e, 0xd82e, 0x242f,
       0x502e, 0xac2f, 0xe82f, 0x142e, 0x0022, 0xfc23, 0xb823, 0x4422, 0x3023, 0xcc22, 0x8822, 0x7423, 0x6020, 0x9c21,
       0xd821, 0x2420, 0x5021, 0xac20, 0xe820, 0x1421, 0xc026, 0x3c27, 0x7827, 0x8426, 0xf027, 0x0c26, 0x4826, 0xb427,
       0xa024, 0x5c25, 0x1825, 0xe424, 0x9025, 0x6c24, 0x2824, 0xd425, 0x003c, 0xfc3d, 0xb83d, 0x443c, 0x303d, 0xcc3c,
       0x883c, 0x743d, 0x603e, 0x9c3f, 0xd83f, 0x243e, 0x503f, 0xac3e, 0xe83e, 0x143f, 0xc038, 0x3c39, 0x7839, 0x8438,
       0xf039, 0x0c38, 0x4838, 0xb439, 0xa03a, 0x5c3b, 0x183b, 0xe43a, 0x903b, 0x6c3a, 0x283a, 0xd43b, 0xc037, 0x3c36,
       0x7836, 0x8437, 0xf036, 0x0c37, 0x4837, 0xb436, 0xa035, 0x5c34, 0x1834, 0xe435, 0x9034, 0x6c35, 0x2835, 0xd434,
       0x0033, 0xfc32, 0xb832, 0x4433, 0x3032, 0xcc33, 0x8833, 0x7432, 0x6031, 0x9c30, 0xd830, 0x2431, 0x5030, 0xac31,
       0xe831, 0x1430};

static inline uint16_t crc16ModbusNext(uint16_t crc, uint8_t data)
{
    return (crc >> 8) ^ crc16ModbusTable[(crc & 0xff) ^ data];
}

static inline uint16_t crc16ModbusNext4(uint16_t crc, uint32_t data)
{
    crc ^= data & 0xffff;
    crc = crc16ModbusTable[(crc & 0xff) + 0x300] ^ crc16ModbusTable[((crc >> 8) & 0xff) + 0x200]
        ^ crc16ModbusTable[((data >> 16) & 0xff) + 0x100] ^ crc16ModbusTable[data >> 24];
    return crc;
}

uint16_t crc16Modbus(const void* data, std::size_t len)
{
    const uint8_t* ptr = (uint8_t*)data;
    uint16_t crc = 0xffff;

    while (((uintptr_t)ptr & 3) && len) {
        len--;
        crc = crc16ModbusNext(crc, *ptr++);
    }

    while (len >= 16) {
        len -= 16;
        crc = crc16ModbusNext4(crc, ((uint32_t*)ptr)[0]);
        crc = crc16ModbusNext4(crc, ((uint32_t*)ptr)[1]);
        crc = crc16ModbusNext4(crc, ((uint32_t*)ptr)[2]);
        crc = crc16ModbusNext4(crc, ((uint32_t*)ptr)[3]);
        ptr += 16;
    }

    while (len) {
        len--;
        crc = crc16ModbusNext(crc, *ptr++);
    }

    return crc;
}

/*
 * CRC32
 *
 * Code generated by universal_crc by Danjel McGougan
 *
 * CRC parameters used:
 *   bits:       32
 *   poly:       0x04c11db7
 *   init:       0xffffffff
 *   xor:        0xffffffff
 *   reverse:    true
 *   non-direct: false
 *
 * CRC of the string "123456789" is 0xcbf43926
 */

const uint32_t crc32Table[1024]
    = {0x00000000, 0x77073096, 0xee0e612c, 0x990951ba, 0x076dc419, 0x706af48f, 0xe963a535, 0x9e6495a3, 0x0edb8832,
       0x79dcb8a4, 0xe0d5e91e, 0x97d2d988, 0x09b64c2b, 0x7eb17cbd, 0xe7b82d07, 0x90bf1d91, 0x1db71064, 0x6ab020f2,
       0xf3b97148, 0x84be41de, 0x1adad47d, 0x6ddde4eb, 0xf4d4b551, 0x83d385c7, 0x136c9856, 0x646ba8c0, 0xfd62f97a,
       0x8a65c9ec, 0x14015c4f, 0x63066cd9, 0xfa0f3d63, 0x8d080df5, 0x3b6e20c8, 0x4c69105e, 0xd56041e4, 0xa2677172,
       0x3c03e4d1, 0x4b04d447, 0xd20d85fd, 0xa50ab56b, 0x35b5a8fa, 0x42b2986c, 0xdbbbc9d6, 0xacbcf940, 0x32d86ce3,
       0x45df5c75, 0xdcd60dcf, 0xabd13d59, 0x26d930ac, 0x51de003a, 0xc8d75180, 0xbfd06116, 0x21b4f4b5, 0x56b3c423,
       0xcfba9599, 0xb8bda50f, 0x2802b89e, 0x5f058808, 0xc60cd9b2, 0xb10be924, 0x2f6f7c87, 0x58684c11, 0xc1611dab,
       0xb6662d3d, 0x76dc4190, 0x01db7106, 0x98d220bc, 0xefd5102a, 0x71b18589, 0x06b6b51f, 0x9fbfe4a5, 0xe8b8d433,
       0x7807c9a2, 0x0f00f934, 0x9609a88e, 0xe10e9818, 0x7f6a0dbb, 0x086d3d2d, 0x91646c97, 0xe6635c01, 0x6b6b51f4,
       0x1c6c6162, 0x856530d8, 0xf262004e, 0x6c0695ed, 0x1b01a57b, 0x8208f4c1, 0xf50fc457, 0x65b0d9c6, 0x12b7e950,
       0x8bbeb8ea, 0xfcb9887c, 0x62dd1ddf, 0x15da2d49, 0x8cd37cf3, 0xfbd44c65, 0x4db26158, 0x3ab551ce, 0xa3bc0074,
       0xd4bb30e2, 0x4adfa541, 0x3dd895d7, 0xa4d1c46d, 0xd3d6f4fb, 0x4369e96a, 0x346ed9fc, 0xad678846, 0xda60b8d0,
       0x44042d73, 0x33031de5, 0xaa0a4c5f, 0xdd0d7cc9, 0x5005713c, 0x270241aa, 0xbe0b1010, 0xc90c2086, 0x5768b525,
       0x206f85b3, 0xb966d409, 0xce61e49f, 0x5edef90e, 0x29d9c998, 0xb0d09822, 0xc7d7a8b4, 0x59b33d17, 0x2eb40d81,
       0xb7bd5c3b, 0xc0ba6cad, 0xedb88320, 0x9abfb3b6, 0x03b6e20c, 0x74b1d29a, 0xead54739, 0x9dd277af, 0x04db2615,
       0x73dc1683, 0xe3630b12, 0x94643b84, 0x0d6d6a3e, 0x7a6a5aa8, 0xe40ecf0b, 0x9309ff9d, 0x0a00ae27, 0x7d079eb1,
       0xf00f9344, 0x8708a3d2, 0x1e01f268, 0x6906c2fe, 0xf762575d, 0x806567cb, 0x196c3671, 0x6e6b06e7, 0xfed41b76,
       0x89d32be0, 0x10da7a5a, 0x67dd4acc, 0xf9b9df6f, 0x8ebeeff9, 0x17b7be43, 0x60b08ed5, 0xd6d6a3e8, 0xa1d1937e,
       0x38d8c2c4, 0x4fdff252, 0xd1bb67f1, 0xa6bc5767, 0x3fb506dd, 0x48b2364b, 0xd80d2bda, 0xaf0a1b4c, 0x36034af6,
       0x41047a60, 0xdf60efc3, 0xa867df55, 0x316e8eef, 0x4669be79, 0xcb61b38c, 0xbc66831a, 0x256fd2a0, 0x5268e236,
       0xcc0c7795, 0xbb0b4703, 0x220216b9, 0x5505262f, 0xc5ba3bbe, 0xb2bd0b28, 0x2bb45a92, 0x5cb36a04, 0xc2d7ffa7,
       0xb5d0cf31, 0x2cd99e8b, 0x5bdeae1d, 0x9b64c2b0, 0xec63f226, 0x756aa39c, 0x026d930a, 0x9c0906a9, 0xeb0e363f,
       0x72076785, 0x05005713, 0x95bf4a82, 0xe2b87a14, 0x7bb12bae, 0x0cb61b38, 0x92d28e9b, 0xe5d5be0d, 0x7cdcefb7,
       0x0bdbdf21, 0x86d3d2d4, 0xf1d4e242, 0x68ddb3f8, 0x1fda836e, 0x81be16cd, 0xf6b9265b, 0x6fb077e1, 0x18b74777,
       0x88085ae6, 0xff0f6a70, 0x66063bca, 0x11010b5c, 0x8f659eff, 0xf862ae69, 0x616bffd3, 0x166ccf45, 0xa00ae278,
       0xd70dd2ee, 0x4e048354, 0x3903b3c2, 0xa7672661, 0xd06016f7, 0x4969474d, 0x3e6e77db, 0xaed16a4a, 0xd9d65adc,
       0x40df0b66, 0x37d83bf0, 0xa9bcae53, 0xdebb9ec5, 0x47b2cf7f, 0x30b5ffe9, 0xbdbdf21c, 0xcabac28a, 0x53b39330,
       0x24b4a3a6, 0xbad03605, 0xcdd70693, 0x54de5729, 0x23d967bf, 0xb3667a2e, 0xc4614ab8, 0x5d681b02, 0x2a6f2b94,
       0xb40bbe37, 0xc30c8ea1, 0x5a05df1b, 0x2d02ef8d, 0x00000000, 0x191b3141, 0x32366282, 0x2b2d53c3, 0x646cc504,
       0x7d77f445, 0x565aa786, 0x4f4196c7, 0xc8d98a08, 0xd1c2bb49, 0xfaefe88a, 0xe3f4d9cb, 0xacb54f0c, 0xb5ae7e4d,
       0x9e832d8e, 0x87981ccf, 0x4ac21251, 0x53d92310, 0x78f470d3, 0x61ef4192, 0x2eaed755, 0x37b5e614, 0x1c98b5d7,
       0x05838496, 0x821b9859, 0x9b00a918, 0xb02dfadb, 0xa936cb9a, 0xe6775d5d, 0xff6c6c1c, 0xd4413fdf, 0xcd5a0e9e,
       0x958424a2, 0x8c9f15e3, 0xa7b24620, 0xbea97761, 0xf1e8e1a6, 0xe8f3d0e7, 0xc3de8324, 0xdac5b265, 0x5d5daeaa,
       0x44469feb, 0x6f6bcc28, 0x7670fd69, 0x39316bae, 0x202a5aef, 0x0b07092c, 0x121c386d, 0xdf4636f3, 0xc65d07b2,
       0xed705471, 0xf46b6530, 0xbb2af3f7, 0xa231c2b6, 0x891c9175, 0x9007a034, 0x179fbcfb, 0x0e848dba, 0x25a9de79,
       0x3cb2ef38, 0x73f379ff, 0x6ae848be, 0x41c51b7d, 0x58de2a3c, 0xf0794f05, 0xe9627e44, 0xc24f2d87, 0xdb541cc6,
       0x94158a01, 0x8d0ebb40, 0xa623e883, 0xbf38d9c2, 0x38a0c50d, 0x21bbf44c, 0x0a96a78f, 0x138d96ce, 0x5ccc0009,
       0x45d73148, 0x6efa628b, 0x77e153ca, 0xbabb5d54, 0xa3a06c15, 0x888d3fd6, 0x91960e97, 0xded79850, 0xc7cca911,
       0xece1fad2, 0xf5facb93, 0x7262d75c, 0x6b79e61d, 0x4054b5de, 0x594f849f, 0x160e1258, 0x0f152319, 0x243870da,
       0x3d23419b, 0x65fd6ba7, 0x7ce65ae6, 0x57cb0925, 0x4ed03864, 0x0191aea3, 0x188a9fe2, 0x33a7cc21, 0x2abcfd60,
       0xad24e1af, 0xb43fd0ee, 0x9f12832d, 0x8609b26c, 0xc94824ab, 0xd05315ea, 0xfb7e4629, 0xe2657768, 0x2f3f79f6,
       0x362448b7, 0x1d091b74, 0x04122a35, 0x4b53bcf2, 0x52488db3, 0x7965de70, 0x607eef31, 0xe7e6f3fe, 0xfefdc2bf,
       0xd5d0917c, 0xcccba03d, 0x838a36fa, 0x9a9107bb, 0xb1bc5478, 0xa8a76539, 0x3b83984b, 0x2298a90a, 0x09b5fac9,
       0x10aecb88, 0x5fef5d4f, 0x46f46c0e, 0x6dd93fcd, 0x74c20e8c, 0xf35a1243, 0xea412302, 0xc16c70c1, 0xd8774180,
       0x9736d747, 0x8e2de606, 0xa500b5c5, 0xbc1b8484, 0x71418a1a, 0x685abb5b, 0x4377e898, 0x5a6cd9d9, 0x152d4f1e,
       0x0c367e5f, 0x271b2d9c, 0x3e001cdd, 0xb9980012, 0xa0833153, 0x8bae6290, 0x92b553d1, 0xddf4c516, 0xc4eff457,
       0xefc2a794, 0xf6d996d5, 0xae07bce9, 0xb71c8da8, 0x9c31de6b, 0x852aef2a, 0xca6b79ed, 0xd37048ac, 0xf85d1b6f,
       0xe1462a2e, 0x66de36e1, 0x7fc507a0, 0x54e85463, 0x4df36522, 0x02b2f3e5, 0x1ba9c2a4, 0x30849167, 0x299fa026,
       0xe4c5aeb8, 0xfdde9ff9, 0xd6f3cc3a, 0xcfe8fd7b, 0x80a96bbc, 0x99b25afd, 0xb29f093e, 0xab84387f, 0x2c1c24b0,
       0x350715f1, 0x1e2a4632, 0x07317773, 0x4870e1b4, 0x516bd0f5, 0x7a468336, 0x635db277, 0xcbfad74e, 0xd2e1e60f,
       0xf9ccb5cc, 0xe0d7848d, 0xaf96124a, 0xb68d230b, 0x9da070c8, 0x84bb4189, 0x03235d46, 0x1a386c07, 0x31153fc4,
       0x280e0e85, 0x674f9842, 0x7e54a903, 0x5579fac0, 0x4c62cb81, 0x8138c51f, 0x9823f45e, 0xb30ea79d, 0xaa1596dc,
       0xe554001b, 0xfc4f315a, 0xd7626299, 0xce7953d8, 0x49e14f17, 0x50fa7e56, 0x7bd72d95, 0x62cc1cd4, 0x2d8d8a13,
       0x3496bb52, 0x1fbbe891, 0x06a0d9d0, 0x5e7ef3ec, 0x4765c2ad, 0x6c48916e, 0x7553a02f, 0x3a1236e8, 0x230907a9,
       0x0824546a, 0x113f652b, 0x96a779e4, 0x8fbc48a5, 0xa4911b66, 0xbd8a2a27, 0xf2cbbce0, 0xebd08da1, 0xc0fdde62,
       0xd9e6ef23, 0x14bce1bd, 0x0da7d0fc, 0x268a833f, 0x3f91b27e, 0x70d024b9, 0x69cb15f8, 0x42e6463b, 0x5bfd777a,
       0xdc656bb5, 0xc57e5af4, 0xee530937, 0xf7483876, 0xb809aeb1, 0xa1129ff0, 0x8a3fcc33, 0x9324fd72, 0x00000000,
       0x01c26a37, 0x0384d46e, 0x0246be59, 0x0709a8dc, 0x06cbc2eb, 0x048d7cb2, 0x054f1685, 0x0e1351b8, 0x0fd13b8f,
       0x0d9785d6, 0x0c55efe1, 0x091af964, 0x08d89353, 0x0a9e2d0a, 0x0b5c473d, 0x1c26a370, 0x1de4c947, 0x1fa2771e,
       0x1e601d29, 0x1b2f0bac, 0x1aed619b, 0x18abdfc2, 0x1969b5f5, 0x1235f2c8, 0x13f798ff, 0x11b126a6, 0x10734c91,
       0x153c5a14, 0x14fe3023, 0x16b88e7a, 0x177ae44d, 0x384d46e0, 0x398f2cd7, 0x3bc9928e, 0x3a0bf8b9, 0x3f44ee3c,
       0x3e86840b, 0x3cc03a52, 0x3d025065, 0x365e1758, 0x379c7d6f, 0x35dac336, 0x3418a901, 0x3157bf84, 0x3095d5b3,
       0x32d36bea, 0x331101dd, 0x246be590, 0x25a98fa7, 0x27ef31fe, 0x262d5bc9, 0x23624d4c, 0x22a0277b, 0x20e69922,
       0x2124f315, 0x2a78b428, 0x2bbade1f, 0x29fc6046, 0x283e0a71, 0x2d711cf4, 0x2cb376c3, 0x2ef5c89a, 0x2f37a2ad,
       0x709a8dc0, 0x7158e7f7, 0x731e59ae, 0x72dc3399, 0x7793251c, 0x76514f2b, 0x7417f172, 0x75d59b45, 0x7e89dc78,
       0x7f4bb64f, 0x7d0d0816, 0x7ccf6221, 0x798074a4, 0x78421e93, 0x7a04a0ca, 0x7bc6cafd, 0x6cbc2eb0, 0x6d7e4487,
       0x6f38fade, 0x6efa90e9, 0x6bb5866c, 0x6a77ec5b, 0x68315202, 0x69f33835, 0x62af7f08, 0x636d153f, 0x612bab66,
       0x60e9c151, 0x65a6d7d4, 0x6464bde3, 0x662203ba, 0x67e0698d, 0x48d7cb20, 0x4915a117, 0x4b531f4e, 0x4a917579,
       0x4fde63fc, 0x4e1c09cb, 0x4c5ab792, 0x4d98dda5, 0x46c49a98, 0x4706f0af, 0x45404ef6, 0x448224c1, 0x41cd3244,
       0x400f5873, 0x4249e62a, 0x438b8c1d, 0x54f16850, 0x55330267, 0x5775bc3e, 0x56b7d609, 0x53f8c08c, 0x523aaabb,
       0x507c14e2, 0x51be7ed5, 0x5ae239e8, 0x5b2053df, 0x5966ed86, 0x58a487b1, 0x5deb9134, 0x5c29fb03, 0x5e6f455a,
       0x5fad2f6d, 0xe1351b80, 0xe0f771b7, 0xe2b1cfee, 0xe373a5d9, 0xe63cb35c, 0xe7fed96b, 0xe5b86732, 0xe47a0d05,
       0xef264a38, 0xeee4200f, 0xeca29e56, 0xed60f461, 0xe82fe2e4, 0xe9ed88d3, 0xebab368a, 0xea695cbd, 0xfd13b8f0,
       0xfcd1d2c7, 0xfe976c9e, 0xff5506a9, 0xfa1a102c, 0xfbd87a1b, 0xf99ec442, 0xf85cae75, 0xf300e948, 0xf2c2837f,
       0xf0843d26, 0xf1465711, 0xf4094194, 0xf5cb2ba3, 0xf78d95fa, 0xf64fffcd, 0xd9785d60, 0xd8ba3757, 0xdafc890e,
       0xdb3ee339, 0xde71f5bc, 0xdfb39f8b, 0xddf521d2, 0xdc374be5, 0xd76b0cd8, 0xd6a966ef, 0xd4efd8b6, 0xd52db281,
       0xd062a404, 0xd1a0ce33, 0xd3e6706a, 0xd2241a5d, 0xc55efe10, 0xc49c9427, 0xc6da2a7e, 0xc7184049, 0xc25756cc,
       0xc3953cfb, 0xc1d382a2, 0xc011e895, 0xcb4dafa8, 0xca8fc59f, 0xc8c97bc6, 0xc90b11f1, 0xcc440774, 0xcd866d43,
       0xcfc0d31a, 0xce02b92d, 0x91af9640, 0x906dfc77, 0x922b422e, 0x93e92819, 0x96a63e9c, 0x976454ab, 0x9522eaf2,
       0x94e080c5, 0x9fbcc7f8, 0x9e7eadcf, 0x9c381396, 0x9dfa79a1, 0x98b56f24, 0x99770513, 0x9b31bb4a, 0x9af3d17d,
       0x8d893530, 0x8c4b5f07, 0x8e0de15e, 0x8fcf8b69, 0x8a809dec, 0x8b42f7db, 0x89044982, 0x88c623b5, 0x839a6488,
       0x82580ebf, 0x801eb0e6, 0x81dcdad1, 0x8493cc54, 0x8551a663, 0x8717183a, 0x86d5720d, 0xa9e2d0a0, 0xa820ba97,
       0xaa6604ce, 0xaba46ef9, 0xaeeb787c, 0xaf29124b, 0xad6fac12, 0xacadc625, 0xa7f18118, 0xa633eb2f, 0xa4755576,
       0xa5b73f41, 0xa0f829c4, 0xa13a43f3, 0xa37cfdaa, 0xa2be979d, 0xb5c473d0, 0xb40619e7, 0xb640a7be, 0xb782cd89,
       0xb2cddb0c, 0xb30fb13b, 0xb1490f62, 0xb08b6555, 0xbbd72268, 0xba15485f, 0xb853f606, 0xb9919c31, 0xbcde8ab4,
       0xbd1ce083, 0xbf5a5eda, 0xbe9834ed, 0x00000000, 0xb8bc6765, 0xaa09c88b, 0x12b5afee, 0x8f629757, 0x37def032,
       0x256b5fdc, 0x9dd738b9, 0xc5b428ef, 0x7d084f8a, 0x6fbde064, 0xd7018701, 0x4ad6bfb8, 0xf26ad8dd, 0xe0df7733,
       0x58631056, 0x5019579f, 0xe8a530fa, 0xfa109f14, 0x42acf871, 0xdf7bc0c8, 0x67c7a7ad, 0x75720843, 0xcdce6f26,
       0x95ad7f70, 0x2d111815, 0x3fa4b7fb, 0x8718d09e, 0x1acfe827, 0xa2738f42, 0xb0c620ac, 0x087a47c9, 0xa032af3e,
       0x188ec85b, 0x0a3b67b5, 0xb28700d0, 0x2f503869, 0x97ec5f0c, 0x8559f0e2, 0x3de59787, 0x658687d1, 0xdd3ae0b4,
       0xcf8f4f5a, 0x7733283f, 0xeae41086, 0x525877e3, 0x40edd80d, 0xf851bf68, 0xf02bf8a1, 0x48979fc4, 0x5a22302a,
       0xe29e574f, 0x7f496ff6, 0xc7f50893, 0xd540a77d, 0x6dfcc018, 0x359fd04e, 0x8d23b72b, 0x9f9618c5, 0x272a7fa0,
       0xbafd4719, 0x0241207c, 0x10f48f92, 0xa848e8f7, 0x9b14583d, 0x23a83f58, 0x311d90b6, 0x89a1f7d3, 0x1476cf6a,
       0xaccaa80f, 0xbe7f07e1, 0x06c36084, 0x5ea070d2, 0xe61c17b7, 0xf4a9b859, 0x4c15df3c, 0xd1c2e785, 0x697e80e0,
       0x7bcb2f0e, 0xc377486b, 0xcb0d0fa2, 0x73b168c7, 0x6104c729, 0xd9b8a04c, 0x446f98f5, 0xfcd3ff90, 0xee66507e,
       0x56da371b, 0x0eb9274d, 0xb6054028, 0xa4b0efc6, 0x1c0c88a3, 0x81dbb01a, 0x3967d77f, 0x2bd27891, 0x936e1ff4,
       0x3b26f703, 0x839a9066, 0x912f3f88, 0x299358ed, 0xb4446054, 0x0cf80731, 0x1e4da8df, 0xa6f1cfba, 0xfe92dfec,
       0x462eb889, 0x549b1767, 0xec277002, 0x71f048bb, 0xc94c2fde, 0xdbf98030, 0x6345e755, 0x6b3fa09c, 0xd383c7f9,
       0xc1366817, 0x798a0f72, 0xe45d37cb, 0x5ce150ae, 0x4e54ff40, 0xf6e89825, 0xae8b8873, 0x1637ef16, 0x048240f8,
       0xbc3e279d, 0x21e91f24, 0x99557841, 0x8be0d7af, 0x335cb0ca, 0xed59b63b, 0x55e5d15e, 0x47507eb0, 0xffec19d5,
       0x623b216c, 0xda874609, 0xc832e9e7, 0x708e8e82, 0x28ed9ed4, 0x9051f9b1, 0x82e4565f, 0x3a58313a, 0xa78f0983,
       0x1f336ee6, 0x0d86c108, 0xb53aa66d, 0xbd40e1a4, 0x05fc86c1, 0x1749292f, 0xaff54e4a, 0x322276f3, 0x8a9e1196,
       0x982bbe78, 0x2097d91d, 0x78f4c94b, 0xc048ae2e, 0xd2fd01c0, 0x6a4166a5, 0xf7965e1c, 0x4f2a3979, 0x5d9f9697,
       0xe523f1f2, 0x4d6b1905, 0xf5d77e60, 0xe762d18e, 0x5fdeb6eb, 0xc2098e52, 0x7ab5e937, 0x680046d9, 0xd0bc21bc,
       0x88df31ea, 0x3063568f, 0x22d6f961, 0x9a6a9e04, 0x07bda6bd, 0xbf01c1d8, 0xadb46e36, 0x15080953, 0x1d724e9a,
       0xa5ce29ff, 0xb77b8611, 0x0fc7e174, 0x9210d9cd, 0x2aacbea8, 0x38191146, 0x80a57623, 0xd8c66675, 0x607a0110,
       0x72cfaefe, 0xca73c99b, 0x57a4f122, 0xef189647, 0xfdad39a9, 0x45115ecc, 0x764dee06, 0xcef18963, 0xdc44268d,
       0x64f841e8, 0xf92f7951, 0x41931e34, 0x5326b1da, 0xeb9ad6bf, 0xb3f9c6e9, 0x0b45a18c, 0x19f00e62, 0xa14c6907,
       0x3c9b51be, 0x842736db, 0x96929935, 0x2e2efe50, 0x2654b999, 0x9ee8defc, 0x8c5d7112, 0x34e11677, 0xa9362ece,
       0x118a49ab, 0x033fe645, 0xbb838120, 0xe3e09176, 0x5b5cf613, 0x49e959fd, 0xf1553e98, 0x6c820621, 0xd43e6144,
       0xc68bceaa, 0x7e37a9cf, 0xd67f4138, 0x6ec3265d, 0x7c7689b3, 0xc4caeed6, 0x591dd66f, 0xe1a1b10a, 0xf3141ee4,
       0x4ba87981, 0x13cb69d7, 0xab770eb2, 0xb9c2a15c, 0x017ec639, 0x9ca9fe80, 0x241599e5, 0x36a0360b, 0x8e1c516e,
       0x866616a7, 0x3eda71c2, 0x2c6fde2c, 0x94d3b949, 0x090481f0, 0xb1b8e695, 0xa30d497b, 0x1bb12e1e, 0x43d23e48,
       0xfb6e592d, 0xe9dbf6c3, 0x516791a6, 0xccb0a91f, 0x740cce7a, 0x66b96194, 0xde0506f1};

static inline uint32_t crc32Next(uint32_t crc, uint8_t data)
{
    return (crc >> 8) ^ crc32Table[(crc & 0xff) ^ data];
}

static inline uint32_t crc32Next4(uint32_t crc, uint32_t data)
{
    crc ^= data;
    crc = crc32Table[(crc & 0xff) + 0x300] ^ crc32Table[((crc >> 8) & 0xff) + 0x200]
        ^ crc32Table[((crc >> 16) & 0xff) + 0x100] ^ crc32Table[(crc >> 24) & 0xff];
    return crc;
}

uint32_t crc32(const void* data, std::size_t len)
{
    const uint8_t* ptr = (uint8_t*)data;
    uint32_t crc = 0xffffffff;

    while (((uintptr_t)ptr & 3) && len) {
        len--;
        crc = crc32Next(crc, *ptr++);
    }

    while (len >= 16) {
        len -= 16;
        crc = crc32Next4(crc, ((uint32_t*)ptr)[0]);
        crc = crc32Next4(crc, ((uint32_t*)ptr)[1]);
        crc = crc32Next4(crc, ((uint32_t*)ptr)[2]);
        crc = crc32Next4(crc, ((uint32_t*)ptr)[3]);
        ptr += 16;
    }

    while (len) {
        len--;
        crc = crc32Next(crc, *ptr++);
    }

    return ~crc;
}

/*
 * CRC16-X25
 *
 * Code generated by universal_crc by Danjel McGougan
 *
 * CRC parameters used:
 *   bits:       16
 *   poly:       0x1021
 *   init:       0xffff
 *   xor:        0xffff
 *   reverse:    true
 *   non-direct: false
 *
 * CRC of the string "123456789" is 0x906e
 */

const uint16_t crc16Mcrf4xxTable[1024]
    = {0x0000, 0x1189, 0x2312, 0x329b, 0x4624, 0x57ad, 0x6536, 0x74bf, 0x8c48, 0x9dc1, 0xaf5a, 0xbed3, 0xca6c, 0xdbe5,
       0xe97e, 0xf8f7, 0x1081, 0x0108, 0x3393, 0x221a, 0x56a5, 0x472c, 0x75b7, 0x643e, 0x9cc9, 0x8d40, 0xbfdb, 0xae52,
       0xdaed, 0xcb64, 0xf9ff, 0xe876, 0x2102, 0x308b, 0x0210, 0x1399, 0x6726, 0x76af, 0x4434, 0x55bd, 0xad4a, 0xbcc3,
       0x8e58, 0x9fd1, 0xeb6e, 0xfae7, 0xc87c, 0xd9f5, 0x3183, 0x200a, 0x1291, 0x0318, 0x77a7, 0x662e, 0x54b5, 0x453c,
       0xbdcb, 0xac42, 0x9ed9, 0x8f50, 0xfbef, 0xea66, 0xd8fd, 0xc974, 0x4204, 0x538d, 0x6116, 0x709f, 0x0420, 0x15a9,
       0x2732, 0x36bb, 0xce4c, 0xdfc5, 0xed5e, 0xfcd7, 0x8868, 0x99e1, 0xab7a, 0xbaf3, 0x5285, 0x430c, 0x7197, 0x601e,
       0x14a1, 0x0528, 0x37b3, 0x263a, 0xdecd, 0xcf44, 0xfddf, 0xec56, 0x98e9, 0x8960, 0xbbfb, 0xaa72, 0x6306, 0x728f,
       0x4014, 0x519d, 0x2522, 0x34ab, 0x0630, 0x17b9, 0xef4e, 0xfec7, 0xcc5c, 0xddd5, 0xa96a, 0xb8e3, 0x8a78, 0x9bf1,
       0x7387, 0x620e, 0x5095, 0x411c, 0x35a3, 0x242a, 0x16b1, 0x0738, 0xffcf, 0xee46, 0xdcdd, 0xcd54, 0xb9eb, 0xa862,
       0x9af9, 0x8b70, 0x8408, 0x9581, 0xa71a, 0xb693, 0xc22c, 0xd3a5, 0xe13e, 0xf0b7, 0x0840, 0x19c9, 0x2b52, 0x3adb,
       0x4e64, 0x5fed, 0x6d76, 0x7cff, 0x9489, 0x8500, 0xb79b, 0xa612, 0xd2ad, 0xc324, 0xf1bf, 0xe036, 0x18c1, 0x0948,
       0x3bd3, 0x2a5a, 0x5ee5, 0x4f6c, 0x7df7, 0x6c7e, 0xa50a, 0xb483, 0x8618, 0x9791, 0xe32e, 0xf2a7, 0xc03c, 0xd1b5,
       0x2942, 0x38cb, 0x0a50, 0x1bd9, 0x6f66, 0x7eef, 0x4c74, 0x5dfd, 0xb58b, 0xa402, 0x9699, 0x8710, 0xf3af, 0xe226,
       0xd0bd, 0xc134, 0x39c3, 0x284a, 0x1ad1, 0x0b58, 0x7fe7, 0x6e6e, 0x5cf5, 0x4d7c, 0xc60c, 0xd785, 0xe51e, 0xf497,
       0x8028, 0x91a1, 0xa33a, 0xb2b3, 0x4a44, 0x5bcd, 0x6956, 0x78df, 0x0c60, 0x1de9, 0x2f72, 0x3efb, 0xd68d, 0xc704,
       0xf59f, 0xe416, 0x90a9, 0x8120, 0xb3bb, 0xa232, 0x5ac5, 0x4b4c, 0x79d7, 0x685e, 0x1ce1, 0x0d68, 0x3ff3, 0x2e7a,
       0xe70e, 0xf687, 0xc41c, 0xd595, 0xa12a, 0xb0a3, 0x8238, 0x93b1, 0x6b46, 0x7acf, 0x4854, 0x59dd, 0x2d62, 0x3ceb,
       0x0e70, 0x1ff9, 0xf78f, 0xe606, 0xd49d, 0xc514, 0xb1ab, 0xa022, 0x92b9, 0x8330, 0x7bc7, 0x6a4e, 0x58d5, 0x495c,
       0x3de3, 0x2c6a, 0x1ef1, 0x0f78, 0x0000, 0x19d8, 0x33b0, 0x2a68, 0x6760, 0x7eb8, 0x54d0, 0x4d08, 0xcec0, 0xd718,
       0xfd70, 0xe4a8, 0xa9a0, 0xb078, 0x9a10, 0x83c8, 0x9591, 0x8c49, 0xa621, 0xbff9, 0xf2f1, 0xeb29, 0xc141, 0xd899,
       0x5b51, 0x4289, 0x68e1, 0x7139, 0x3c31, 0x25e9, 0x0f81, 0x1659, 0x2333, 0x3aeb, 0x1083, 0x095b, 0x4453, 0x5d8b,
       0x77e3, 0x6e3b, 0xedf3, 0xf42b, 0xde43, 0xc79b, 0x8a93, 0x934b, 0xb923, 0xa0fb, 0xb6a2, 0xaf7a, 0x8512, 0x9cca,
       0xd1c2, 0xc81a, 0xe272, 0xfbaa, 0x7862, 0x61ba, 0x4bd2, 0x520a, 0x1f02, 0x06da, 0x2cb2, 0x356a, 0x4666, 0x5fbe,
       0x75d6, 0x6c0e, 0x2106, 0x38de, 0x12b6, 0x0b6e, 0x88a6, 0x917e, 0xbb16, 0xa2ce, 0xefc6, 0xf61e, 0xdc76, 0xc5ae,
       0xd3f7, 0xca2f, 0xe047, 0xf99f, 0xb497, 0xad4f, 0x8727, 0x9eff, 0x1d37, 0x04ef, 0x2e87, 0x375f, 0x7a57, 0x638f,
       0x49e7, 0x503f, 0x6555, 0x7c8d, 0x56e5, 0x4f3d, 0x0235, 0x1bed, 0x3185, 0x285d, 0xab95, 0xb24d, 0x9825, 0x81fd,
       0xccf5, 0xd52d, 0xff45, 0xe69d, 0xf0c4, 0xe91c, 0xc374, 0xdaac, 0x97a4, 0x8e7c, 0xa414, 0xbdcc, 0x3e04, 0x27dc,
       0x0db4, 0x146c, 0x5964, 0x40bc, 0x6ad4, 0x730c, 0x8ccc, 0x9514, 0xbf7c, 0xa6a4, 0xebac, 0xf274, 0xd81c, 0xc1c4,
       0x420c, 0x5bd4, 0x71bc, 0x6864, 0x256c, 0x3cb4, 0x16dc, 0x0f04, 0x195d, 0x0085, 0x2aed, 0x3335, 0x7e3d, 0x67e5,
       0x4d8d, 0x5455, 0xd79d, 0xce45, 0xe42d, 0xfdf5, 0xb0fd, 0xa925, 0x834d, 0x9a95, 0xafff, 0xb627, 0x9c4f, 0x8597,
       0xc89f, 0xd147, 0xfb2f, 0xe2f7, 0x613f, 0x78e7, 0x528f, 0x4b57, 0x065f, 0x1f87, 0x35ef, 0x2c37, 0x3a6e, 0x23b6,
       0x09de, 0x1006, 0x5d0e, 0x44d6, 0x6ebe, 0x7766, 0xf4ae, 0xed76, 0xc71e, 0xdec6, 0x93ce, 0x8a16, 0xa07e, 0xb9a6,
       0xcaaa, 0xd372, 0xf91a, 0xe0c2, 0xadca, 0xb412, 0x9e7a, 0x87a2, 0x046a, 0x1db2, 0x37da, 0x2e02, 0x630a, 0x7ad2,
       0x50ba, 0x4962, 0x5f3b, 0x46e3, 0x6c8b, 0x7553, 0x385b, 0x2183, 0x0beb, 0x1233, 0x91fb, 0x8823, 0xa24b, 0xbb93,
       0xf69b, 0xef43, 0xc52b, 0xdcf3, 0xe999, 0xf041, 0xda29, 0xc3f1, 0x8ef9, 0x9721, 0xbd49, 0xa491, 0x2759, 0x3e81,
       0x14e9, 0x0d31, 0x4039, 0x59e1, 0x7389, 0x6a51, 0x7c08, 0x65d0, 0x4fb8, 0x5660, 0x1b68, 0x02b0, 0x28d8, 0x3100,
       0xb2c8, 0xab10, 0x8178, 0x98a0, 0xd5a8, 0xcc70, 0xe618, 0xffc0, 0x0000, 0x5adc, 0xb5b8, 0xef64, 0x6361, 0x39bd,
       0xd6d9, 0x8c05, 0xc6c2, 0x9c1e, 0x737a, 0x29a6, 0xa5a3, 0xff7f, 0x101b, 0x4ac7, 0x8595, 0xdf49, 0x302d, 0x6af1,
       0xe6f4, 0xbc28, 0x534c, 0x0990, 0x4357, 0x198b, 0xf6ef, 0xac33, 0x2036, 0x7aea, 0x958e, 0xcf52, 0x033b, 0x59e7,
       0xb683, 0xec5f, 0x605a, 0x3a86, 0xd5e2, 0x8f3e, 0xc5f9, 0x9f25, 0x7041, 0x2a9d, 0xa698, 0xfc44, 0x1320, 0x49fc,
       0x86ae, 0xdc72, 0x3316, 0x69ca, 0xe5cf, 0xbf13, 0x5077, 0x0aab, 0x406c, 0x1ab0, 0xf5d4, 0xaf08, 0x230d, 0x79d1,
       0x96b5, 0xcc69, 0x0676, 0x5caa, 0xb3ce, 0xe912, 0x6517, 0x3fcb, 0xd0af, 0x8a73, 0xc0b4, 0x9a68, 0x750c, 0x2fd0,
       0xa3d5, 0xf909, 0x166d, 0x4cb1, 0x83e3, 0xd93f, 0x365b, 0x6c87, 0xe082, 0xba5e, 0x553a, 0x0fe6, 0x4521, 0x1ffd,
       0xf099, 0xaa45, 0x2640, 0x7c9c, 0x93f8, 0xc924, 0x054d, 0x5f91, 0xb0f5, 0xea29, 0x662c, 0x3cf0, 0xd394, 0x8948,
       0xc38f, 0x9953, 0x7637, 0x2ceb, 0xa0ee, 0xfa32, 0x1556, 0x4f8a, 0x80d8, 0xda04, 0x3560, 0x6fbc, 0xe3b9, 0xb965,
       0x5601, 0x0cdd, 0x461a, 0x1cc6, 0xf3a2, 0xa97e, 0x257b, 0x7fa7, 0x90c3, 0xca1f, 0x0cec, 0x5630, 0xb954, 0xe388,
       0x6f8d, 0x3551, 0xda35, 0x80e9, 0xca2e, 0x90f2, 0x7f96, 0x254a, 0xa94f, 0xf393, 0x1cf7, 0x462b, 0x8979, 0xd3a5,
       0x3cc1, 0x661d, 0xea18, 0xb0c4, 0x5fa0, 0x057c, 0x4fbb, 0x1567, 0xfa03, 0xa0df, 0x2cda, 0x7606, 0x9962, 0xc3be,
       0x0fd7, 0x550b, 0xba6f, 0xe0b3, 0x6cb6, 0x366a, 0xd90e, 0x83d2, 0xc915, 0x93c9, 0x7cad, 0x2671, 0xaa74, 0xf0a8,
       0x1fcc, 0x4510, 0x8a42, 0xd09e, 0x3ffa, 0x6526, 0xe923, 0xb3ff, 0x5c9b, 0x0647, 0x4c80, 0x165c, 0xf938, 0xa3e4,
       0x2fe1, 0x753d, 0x9a59, 0xc085, 0x0a9a, 0x5046, 0xbf22, 0xe5fe, 0x69fb, 0x3327, 0xdc43, 0x869f, 0xcc58, 0x9684,
       0x79e0, 0x233c, 0xaf39, 0xf5e5, 0x1a81, 0x405d, 0x8f0f, 0xd5d3, 0x3ab7, 0x606b, 0xec6e, 0xb6b2, 0x59d6, 0x030a,
       0x49cd, 0x1311, 0xfc75, 0xa6a9, 0x2aac, 0x7070, 0x9f14, 0xc5c8, 0x09a1, 0x537d, 0xbc19, 0xe6c5, 0x6ac0, 0x301c,
       0xdf78, 0x85a4, 0xcf63, 0x95bf, 0x7adb, 0x2007, 0xac02, 0xf6de, 0x19ba, 0x4366, 0x8c34, 0xd6e8, 0x398c, 0x6350,
       0xef55, 0xb589, 0x5aed, 0x0031, 0x4af6, 0x102a, 0xff4e, 0xa592, 0x2997, 0x734b, 0x9c2f, 0xc6f3, 0x0000, 0x1cbb,
       0x3976, 0x25cd, 0x72ec, 0x6e57, 0x4b9a, 0x5721, 0xe5d8, 0xf963, 0xdcae, 0xc015, 0x9734, 0x8b8f, 0xae42, 0xb2f9,
       0xc3a1, 0xdf1a, 0xfad7, 0xe66c, 0xb14d, 0xadf6, 0x883b, 0x9480, 0x2679, 0x3ac2, 0x1f0f, 0x03b4, 0x5495, 0x482e,
       0x6de3, 0x7158, 0x8f53, 0x93e8, 0xb625, 0xaa9e, 0xfdbf, 0xe104, 0xc4c9, 0xd872, 0x6a8b, 0x7630, 0x53fd, 0x4f46,
       0x1867, 0x04dc, 0x2111, 0x3daa, 0x4cf2, 0x5049, 0x7584, 0x693f, 0x3e1e, 0x22a5, 0x0768, 0x1bd3, 0xa92a, 0xb591,
       0x905c, 0x8ce7, 0xdbc6, 0xc77d, 0xe2b0, 0xfe0b, 0x16b7, 0x0a0c, 0x2fc1, 0x337a, 0x645b, 0x78e0, 0x5d2d, 0x4196,
       0xf36f, 0xefd4, 0xca19, 0xd6a2, 0x8183, 0x9d38, 0xb8f5, 0xa44e, 0xd516, 0xc9ad, 0xec60, 0xf0db, 0xa7fa, 0xbb41,
       0x9e8c, 0x8237, 0x30ce, 0x2c75, 0x09b8, 0x1503, 0x4222, 0x5e99, 0x7b54, 0x67ef, 0x99e4, 0x855f, 0xa092, 0xbc29,
       0xeb08, 0xf7b3, 0xd27e, 0xcec5, 0x7c3c, 0x6087, 0x454a, 0x59f1, 0x0ed0, 0x126b, 0x37a6, 0x2b1d, 0x5a45, 0x46fe,
       0x6333, 0x7f88, 0x28a9, 0x3412, 0x11df, 0x0d64, 0xbf9d, 0xa326, 0x86eb, 0x9a50, 0xcd71, 0xd1ca, 0xf407, 0xe8bc,
       0x2d6e, 0x31d5, 0x1418, 0x08a3, 0x5f82, 0x4339, 0x66f4, 0x7a4f, 0xc8b6, 0xd40d, 0xf1c0, 0xed7b, 0xba5a, 0xa6e1,
       0x832c, 0x9f97, 0xeecf, 0xf274, 0xd7b9, 0xcb02, 0x9c23, 0x8098, 0xa555, 0xb9ee, 0x0b17, 0x17ac, 0x3261, 0x2eda,
       0x79fb, 0x6540, 0x408d, 0x5c36, 0xa23d, 0xbe86, 0x9b4b, 0x87f0, 0xd0d1, 0xcc6a, 0xe9a7, 0xf51c, 0x47e5, 0x5b5e,
       0x7e93, 0x6228, 0x3509, 0x29b2, 0x0c7f, 0x10c4, 0x619c, 0x7d27, 0x58ea, 0x4451, 0x1370, 0x0fcb, 0x2a06, 0x36bd,
       0x8444, 0x98ff, 0xbd32, 0xa189, 0xf6a8, 0xea13, 0xcfde, 0xd365, 0x3bd9, 0x2762, 0x02af, 0x1e14, 0x4935, 0x558e,
       0x7043, 0x6cf8, 0xde01, 0xc2ba, 0xe777, 0xfbcc, 0xaced, 0xb056, 0x959b, 0x8920, 0xf878, 0xe4c3, 0xc10e, 0xddb5,
       0x8a94, 0x962f, 0xb3e2, 0xaf59, 0x1da0, 0x011b, 0x24d6, 0x386d, 0x6f4c, 0x73f7, 0x563a, 0x4a81, 0xb48a, 0xa831,
       0x8dfc, 0x9147, 0xc666, 0xdadd, 0xff10, 0xe3ab, 0x5152, 0x4de9, 0x6824, 0x749f, 0x23be, 0x3f05, 0x1ac8, 0x0673,
       0x772b, 0x6b90, 0x4e5d, 0x52e6, 0x05c7, 0x197c, 0x3cb1, 0x200a, 0x92f3, 0x8e48, 0xab85, 0xb73e, 0xe01f, 0xfca4,
       0xd969, 0xc5d2};

static inline uint16_t crc16Mcrf4xxNext(uint16_t crc, uint8_t data)
{
    return (crc >> 8) ^ crc16Mcrf4xxTable[(crc & 0xff) ^ data];
}

static inline uint16_t crc16Mcrf4xxNext4(uint16_t crc, uint32_t data)
{
    crc ^= data & 0xffff;
    crc = crc16Mcrf4xxTable[(crc & 0xff) + 0x300] ^ crc16Mcrf4xxTable[((crc >> 8) & 0xff) + 0x200]
        ^ crc16Mcrf4xxTable[((data >> 16) & 0xff) + 0x100] ^ crc16Mcrf4xxTable[data >> 24];
    return crc;
}

uint16_t crc16Mcrf4xx(const void* data, std::size_t len)
{
    const uint8_t* ptr = (uint8_t*)data;
    uint16_t crc = 0xffff;

    while (((uintptr_t)ptr & 3) && len) {
        crc = crc16Mcrf4xxNext(crc, *ptr++);
        len--;
    }

    while (len >= 16) {
        len -= 16;
        crc = crc16Mcrf4xxNext4(crc, ((uint32_t*)ptr)[0]);
        crc = crc16Mcrf4xxNext4(crc, ((uint32_t*)ptr)[1]);
        crc = crc16Mcrf4xxNext4(crc, ((uint32_t*)ptr)[2]);
        crc = crc16Mcrf4xxNext4(crc, ((uint32_t*)ptr)[3]);
        ptr += 16;
    }

    while (len--) {
        crc = crc16Mcrf4xxNext(crc, *ptr++);
    }

    return crc;
}

uint16_t crc16X25(const void* data, std::size_t len)
{
    return ~crc16Mcrf4xx(data, len);
}
}
}