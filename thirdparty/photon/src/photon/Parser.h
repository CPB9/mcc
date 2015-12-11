#ifndef __PHOTON_PARSER__
#define __PHOTON_PARSER__

#include "photon/Result.h"
#include "photon/Reader.h"
#include "photon/Decoder.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef PhotonResult (*PhotonHandler)(PhotonReader*);

typedef struct {
    PhotonResult (*handleExchangePacket)(void* data, const PhotonExchangePacket*);
    PhotonResult (*handleCounterAdjustmentPacket)(void* data, const PhotonCounterAdjustmentPacket*);
    PhotonResult (*handleReceiptPacket)(void* data, const PhotonReceiptPacket*);
    PhotonResult (*handleAddressPacket)(void* data, const PhotonAddressPacket*);
    PhotonResult (*handleTmEventMessage)(void* data, const PhotonTmEventMessage*);
    PhotonResult (*handleTmStatusMessage)(void* data, const PhotonTmStatusMessage*);
    PhotonResult (*handleCommandMessage)(void* data, const PhotonCommandMessage*);
    PhotonResult (*handleCommandResult)(void* data, const PhotonCommandResult*);
    void* data;
    PhotonStreamType type;
} PhotonParser;

PhotonResult PhotonParser_ParseMultiplexStream(PhotonParser* self, PhotonReader* src);
PhotonResult PhotonParser_ParseExchangeStream(PhotonParser* self, PhotonReader* src);
PhotonResult PhotonParser_ParseApplicationStream(PhotonParser* self, PhotonReader* src);
PhotonResult PhotonParser_ParseCommandStream(PhotonParser* self, PhotonReader* src);
PhotonResult PhotonParser_ParseResultStream(PhotonParser* self, PhotonReader* src);
PhotonResult PhotonParser_ParseTelemetryStream(PhotonParser* self, PhotonReader* src);

#ifdef __cplusplus
}
#endif

#endif
