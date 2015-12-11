#include "photon/RingBuffer.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>

void PhotonRingBuf_Init(PhotonRingBuf* self, void* data, size_t size)
{
    assert(size > 0);
    self->data = (uint8_t*)data;
    self->size = size;
    self->freeSpace = size;
    self->readOffset = 0;
    self->writeOffset = 0;
}

void PhotonRingBuf_Peek(const PhotonRingBuf* self, void* dest, size_t size, size_t offset)
{
    size_t readOffset = self->readOffset + offset;
    if (readOffset >= self->size) {
        readOffset -= self->size;
    }
    assert(size > 0);
    assert(size + offset <= self->size - self->freeSpace);
    if (readOffset < self->writeOffset) { /* ----------r**************w---------- */
        memcpy(dest, self->data + readOffset, size);
    } else { /* *********w---------------r************ */
        size_t rightData = self->size - readOffset;
        size_t firstChunkSize = min(size, rightData);
        memcpy(dest, self->data + readOffset, firstChunkSize);
        if (size > firstChunkSize) {
            size_t secondChunkSize = size - firstChunkSize;
            memcpy((uint8_t*)dest + firstChunkSize, self->data, secondChunkSize);
        }
    }
}

void PhotonRingBuf_Erase(PhotonRingBuf* self, size_t size)
{
    assert(self->size - self->freeSpace >= size);
    self->freeSpace += size;
    self->readOffset += size;
    if (self->readOffset >= self->size) {
        self->readOffset -= self->size;
    }
}

void PhotonRingBuf_Read(PhotonRingBuf* self, void* dest, size_t size)
{
    PhotonRingBuf_Peek(self, dest, size, 0);
    PhotonRingBuf_Erase(self, size);
}

static void extend(PhotonRingBuf* self, size_t size)
{
    self->writeOffset += size;
    if (self->writeOffset >= self->size) {
        self->writeOffset -= self->size;
    }
    self->freeSpace -= size;
}

void PhotonRingBuf_Write(PhotonRingBuf* self, const void* data, size_t size)
{
    assert(size > 0);
    assert(size <= self->size);
    if (self->freeSpace < size) {
        PhotonRingBuf_Erase(self, size - self->freeSpace);
    }
    if (self->readOffset > self->writeOffset) { /* *********w---------------r************ */
        memcpy(self->data + self->writeOffset, data, size);
    } else { /* ----------r**************w---------- */
        size_t rightData = self->size - self->writeOffset;
        size_t firstChunkSize = min(size, rightData);
        memcpy(self->data + self->writeOffset, data, firstChunkSize);
        if (size > firstChunkSize) {
            size_t secondChunkSize = size - firstChunkSize;
            memcpy(self->data, (const uint8_t*)data + firstChunkSize, secondChunkSize);
        }
    }
    extend(self, size);
}

size_t PhotonRingBuf_ReadableSize(const PhotonRingBuf* self)
{
    return self->freeSpace;
}
