#include "photon/Reader.h"

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>

void PhotonReader_Init(PhotonReader* self, const void* data, size_t size)
{
    self->start = data;
    self->current = data;
    self->end = self->start + size;
}

PhotonResult PhotonReader_ReadBer(PhotonReader* self, PhotonBerValue* dest)
{
    if (self->current >= self->end) {
        return PhotonResult_UnexpectedEndOfBerStream;
    }

    uint8_t firstOctet = *(uint8_t*)self->current;
    if (!(firstOctet & 0x80)) {
        self->current++;
        *dest = firstOctet;
        return PhotonResult_Ok;
    }

    unsigned size = firstOctet & 0x7f;
    if ((size > 8) || (size == 0)) {
        return PhotonResult_InvalidBerLength;
    }
    if ((self->end - self->current) < (size + 1)) {
        return PhotonResult_UnexpectedEndOfBerStream;
    }

    *dest = 0;
    memcpy(dest, self->current + 1, size); // big endian?
    self->current += size + 1;
    return PhotonResult_Ok;
}

bool PhotonReader_IsAtEnd(const PhotonReader* self)
{
    return self->current == self->end;
}
