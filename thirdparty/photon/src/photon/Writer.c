#include "photon/Writer.h"

#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>

void PhotonReader_Init(PhotonWriter* self, void* dest, size_t size)
{
    self->start = dest;
    self->current = dest;
    self->end = self->start + size;
}

PhotonResult PhotonWriter_WriteBer(PhotonWriter* self, uint64_t value)
{
    if (value < 128) {
        if (self->current == self->end) {
            return PhotonResult_NotEnoughSpace;
        }
        *self->current = (uint8_t)value;
        self->current++;
        return PhotonResult_Ok;
    }

    unsigned size;
    if (value < 256) {
        size = 1;
    } else if (value < 65536) {
        size = 2;
    } else if (value < 16777216) {
        size = 3;
    } else if (value < 4294967296) {
        size = 4;
    } else if (value < 1099511627776) {
        size = 5;
    } else if (value < 281474976710656) {
        size = 6;
    } else if (value < 72057594037927936) {
        size = 7;
    } else {
        size = 8;
    }

    if ((self->end - self->current) < (size + 1)) {
        return PhotonResult_NotEnoughSpace;
    }

    *self->current = 0x80 & size;
    memcpy(self->current + 1, &value, size); // big endian?
    self->current += size + 1;
    return PhotonResult_Ok;
}


bool PhotonWriter_IsAtEnd(const PhotonWriter* self)
{
    return self->current == self->end;
}
