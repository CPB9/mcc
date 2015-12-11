#ifndef __PHOTON_ENCODER__
#define __PHOTON_ENCODER__

#include "photon/Result.h"
#include "photon/Enums.h"
#include "photon/Writer.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef uint64_t PhotonBerValue;

typedef PhotonResult (*PhotonGenerator)(void* data, PhotonWriter* dest);

typedef struct {
    PhotonBerValue segmentNumber;
    PhotonBerValue maxSegmentNumber;
    PhotonBerValue componentNumber;
    PhotonBerValue messageNumber;
    void* data;
    PhotonGenerator gen;
} PhotonTmStatusMessageGen;

typedef struct {
    PhotonBerValue componentNumber;
    PhotonBerValue messageNumber;
    PhotonBerValue eventNumber;
    PhotonBerValue timestamp;
    void* data;
    PhotonGenerator gen;
} PhotonTmEventMessageGen;

typedef struct {
    PhotonBerValue srcAddress;
    PhotonBerValue srcComponentNumber;
    PhotonBerValue destComponentNumber;
    PhotonBerValue destAddress;
    PhotonBerValue srcGroup;
    PhotonBerValue destGroup;
} PhotonGroupAddress;

typedef struct {
    PhotonBerValue srcAddress;
    PhotonBerValue srcComponentNumber;
    PhotonBerValue destComponentNumber;
    PhotonBerValue destAddress;
} PhotonNetworkAddress;

typedef struct {
    PhotonBerValue srcAddress;
    PhotonBerValue srcComponentNumber;
    PhotonBerValue destComponentNumber;
} PhotonMulticastAddress;

typedef struct {
    PhotonAddressType addressType;
    PhotonBerValue srcAddress;
    PhotonBerValue srcComponentNumber;
    PhotonBerValue destComponentNumber;
    PhotonBerValue destAddress;
    PhotonBerValue srcGroup;
    PhotonBerValue destGroup;
    PhotonBerValue timestampType;
    PhotonBerValue timestamp;
    void* data;
    PhotonGenerator gen;
} PhotonAddressPacketEnc;

typedef struct {
    PhotonErrorControlType errorControlType;
    PhotonBerValue lastSequenceCounter;
    void* data;
    PhotonGenerator gen;
} PhotonReceiptPacketEnc;

typedef struct {
    PhotonStreamType streamType;
    PhotonErrorControlType errorControlType;
    PhotonBerValue sequenceCounter;
    void* data;
    PhotonGenerator gen;
} PhotonCounterAdjustmentPacketEnc;

typedef struct {
    PhotonStreamType streamType;
    PhotonErrorControlType errorControlType;
    PhotonBerValue windowSize;
    PhotonBerValue sequenceCounter;
    void* data;
    PhotonGenerator gen;
} PhotonExchangePacketEnc;

PhotonResult PhotonEncoder_EncodeData(uint16_t header, void* data, PhotonGenerator dataGen, PhotonWriter* dest);
PhotonResult PhotonEncoder_EncodeCommandResult(void* data, PhotonGenerator resultGen, PhotonWriter* dest);
PhotonResult PhotonEncoder_EncodeCommandMessage(void* data, PhotonGenerator msgGen, PhotonWriter* dest);
PhotonResult PhotonEncoder_EncodeTmStatusMessage(PhotonTmStatusMessageGen* gen, PhotonWriter* dest);
PhotonResult PhotonEncoder_EncodeTmEventMessage(PhotonTmEventMessageGen* gen, PhotonWriter* dest);
PhotonResult PhotonEncoder_EncodeAddressPacket(PhotonAddressPacketEnc* encoder, PhotonWriter* dest);
PhotonResult PhotonEncoder_EncodeReceiptPacket(PhotonReceiptPacketEnc* encoder, PhotonWriter* dest);
PhotonResult PhotonEncoder_EncodeCounterAdjustmentPacket(PhotonCounterAdjustmentPacketEnc* encoder, PhotonWriter* dest);
PhotonResult PhotonEncoder_EncodeExchangePacket(PhotonExchangePacketEnc* encoder, PhotonWriter* dest);

#ifdef __cplusplus
}
#endif

#endif
