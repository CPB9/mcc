#ifndef __PHOTON_WRITER__
#define __PHOTON_WRITER__

#include "photon/Result.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    uint8_t* current;
    uint8_t* start;
    const uint8_t* end;
} PhotonWriter;

void PhotonWriter_Init(PhotonWriter* self, void* dest, size_t size);

PhotonResult PhotonWriter_WriteBer(PhotonWriter* self, uint64_t value);
void PhotonWriter_WriteUint16Be(PhotonWriter* self, uint16_t value);
void PhotonWriter_WriteUint32Be(PhotonWriter* self, uint16_t value);
void PhotonWriter_Skip(PhotonWriter* self, size_t size);

void PhotonWriter_SliceFromBack(PhotonWriter* self, size_t length, PhotonWriter* dest);

uint8_t* PhotonWriter_CurrentPtr(const PhotonWriter* self);

bool PhotonWriter_IsAtEnd(const PhotonWriter* self);
size_t PhotonWriter_WritableSize(const PhotonWriter* self);

#ifdef __cplusplus
}
#endif

#endif
