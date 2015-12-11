#ifndef __PHOTON_RINGBUFFER__
#define __PHOTON_RINGBUFFER__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef struct {
    uint8_t* data;
    size_t readOffset;
    size_t writeOffset;
    size_t size;
    size_t freeSpace;
} PhotonRingBuf;

void PhotonRingBuf_Init(PhotonRingBuf* self, void* data, size_t size);
void PhotonRingBuf_Peek(const PhotonRingBuf* self, void* dest, size_t size, size_t offset);
uint8_t PhotonRingBuf_PeekUint8(const PhotonRingBuf* self, size_t offset);
void PhotonRingBuf_Read(PhotonRingBuf* self, void* dest, size_t size);
void PhotonRingBuf_Skip(PhotonRingBuf* self, size_t size);
void PhotonRingBuf_Erase(PhotonRingBuf* self, size_t size);
void PhotonRingBuf_Write(PhotonRingBuf* self, const void* src, size_t size);
size_t PhotonRingBuf_ReadableSize(const PhotonRingBuf* self);

#ifdef __cplusplus
}
#endif

#endif
