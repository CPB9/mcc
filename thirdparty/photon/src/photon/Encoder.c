#include "photon/Encoder.h"
#include "photon/Result.h"

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>

static void writeBer16Fixed(uint16_t value, uint8_t* dest)
{
    *dest = 0x80 & 3;
    *(dest + 1) = (value & 0xf0) >> 8;
    *(dest + 2) = value & 0x0f;
}

PhotonResult PhotonEncoder_EncodeData(uint16_t header, void* data, PhotonGenerator gen, PhotonWriter* dest)
{
    if (PhotonWriter_WritableSize(dest) < 5) {
        return PhotonResult_NotEnoughSpace;
    }

    PhotonWriter_WriteUint16Be(dest, header);

    uint8_t* sizeDest = PhotonWriter_CurrentPtr(dest);
    PhotonWriter_Skip(dest, 3);

    PHOTON_TRY(gen(data, dest));

    size_t dataSize = PhotonWriter_CurrentPtr(dest) - sizeDest;
    writeBer16Fixed(dataSize, sizeDest);

    return PhotonResult_Ok;
}

PhotonResult PhotonEncoder_EncodeCommandResult(void* data, PhotonGenerator resultGen, PhotonWriter* dest)
{
    return PhotonEncoder_EncodeData(0x0c66, data, resultGen, dest);
}

PhotonResult PhotonEncoder_EncodeCommandMessage(void* data, PhotonGenerator msgGen, PhotonWriter* dest)
{
    return PhotonEncoder_EncodeData(0x0c65, data, msgGen, dest);
}

static PhotonResult tmStatusMessageGen(void* data, PhotonWriter* dest)
{
    PhotonTmStatusMessageGen* gen = (PhotonTmStatusMessageGen*)data;
    PHOTON_TRY(PhotonWriter_WriteBer(dest, gen->segmentNumber));
    PHOTON_TRY(PhotonWriter_WriteBer(dest, gen->maxSegmentNumber));
    PHOTON_TRY(PhotonWriter_WriteBer(dest, gen->componentNumber));
    PHOTON_TRY(PhotonWriter_WriteBer(dest, gen->messageNumber));
    return gen->gen(gen->data, dest);
}

PhotonResult PhotonEncoder_EncodeTmStatusMessage(PhotonTmStatusMessageGen* gen, PhotonWriter* dest)
{
    return PhotonEncoder_EncodeData(0x1c72, gen, tmStatusMessageGen, dest);
}

static PhotonResult tmEventMessageGen(void* data, PhotonWriter* dest)
{
    PhotonTmEventMessageGen* gen = (PhotonTmEventMessageGen*)data;
    PHOTON_TRY(PhotonWriter_WriteBer(dest, gen->componentNumber));
    PHOTON_TRY(PhotonWriter_WriteBer(dest, gen->messageNumber));
    PHOTON_TRY(PhotonWriter_WriteBer(dest, gen->eventNumber));
    PHOTON_TRY(PhotonWriter_WriteBer(dest, gen->timestamp));
    return gen->gen(gen->data, dest);
}

PhotonResult PhotonEncoder_EncodeTmEventMessage(PhotonTmEventMessageGen* gen, PhotonWriter* dest)
{
    return PhotonEncoder_EncodeData(0x0c78, gen, tmEventMessageGen, dest);
}

static PhotonResult addressPacketGen(void* data, PhotonWriter* dest)
{
    PhotonAddressPacketEnc* gen = (PhotonAddressPacketEnc*)data;
    PHOTON_TRY(PhotonWriter_WriteBer(dest, (PhotonBerValue)gen->addressType));
    PHOTON_TRY(PhotonWriter_WriteBer(dest, gen->srcAddress));
    PHOTON_TRY(PhotonWriter_WriteBer(dest, gen->srcComponentNumber));
    PHOTON_TRY(PhotonWriter_WriteBer(dest, gen->destComponentNumber));

    switch (gen->addressType) {
    case PhotonAddressType_Broadcast:
        break;
    case PhotonAddressType_NetworkAddress:
        PHOTON_TRY(PhotonWriter_WriteBer(dest, gen->destAddress));
        break;
    case PhotonAddressType_GroupAddress:
        PHOTON_TRY(PhotonWriter_WriteBer(dest, gen->destAddress));
        PHOTON_TRY(PhotonWriter_WriteBer(dest, gen->srcGroup));
        PHOTON_TRY(PhotonWriter_WriteBer(dest, gen->destGroup));
        break;
    default:
        return PhotonResult_InvalidAddressType;
    }

    PHOTON_TRY(PhotonWriter_WriteBer(dest, 2)); // TODO: енум метки времени
    PHOTON_TRY(PhotonWriter_WriteBer(dest, gen->timestamp));

    return gen->gen(gen->data, dest);
}

PhotonResult PhotonEncoder_EncodeAddressPacket(PhotonAddressPacketEnc* encoder, PhotonWriter* dest)
{
    return PhotonEncoder_EncodeData(0xac5e, encoder, addressPacketGen, dest);
}


static PhotonResult encodePacket(uint16_t header, PhotonErrorControlType csType, void* data, PhotonGenerator gen,
                                 PhotonWriter* dest)
{
    uint8_t* start = PhotonWriter_CurrentPtr(dest);
    if (PhotonWriter_WritableSize(dest) < 2) {
        return PhotonResult_NotEnoughSpace;
    }

    PhotonWriter_WriteUint16Be(dest, header);

    PhotonWriter payload;
    int csSize;
    switch (csType) {
    case PhotonErrorControlType_Crc16:
        csSize = 2;
        break;
    case PhotonErrorControlType_ReedSolomon:
        csSize = 4;
        break;
    default:
        return PhotonResult_InvalidErrorControlType;
    };

    if ((dest->end - dest->current) < csSize) {
        return PhotonResult_NotEnoughSpace;
    }
    PhotonWriter_SliceFromBack(dest, csSize, &payload);

    PHOTON_TRY(gen(data, &payload));
    dest->current = payload.current;

    // TODO: контрольные суммы
    switch (csType) {
    case PhotonErrorControlType_Crc16:
        (void)start;
        uint16_t crc16 = 0;
        PhotonWriter_WriteUint16Be(dest, crc16);
        break;
    case PhotonErrorControlType_ReedSolomon:
        (void)start;
        uint32_t rs = 0;
        PhotonWriter_WriteUint32Be(dest, rs);
        break;
    };

    return PhotonResult_Ok;
}

static PhotonResult receiptPacketGen(void* data, PhotonWriter* dest)
{
    PhotonReceiptPacketEnc* gen = (PhotonReceiptPacketEnc*)data;
    PHOTON_TRY(PhotonWriter_WriteBer(dest, 1));
    PHOTON_TRY(PhotonWriter_WriteBer(dest, (PhotonBerValue)PhotonStreamType_Commands));
    PHOTON_TRY(PhotonWriter_WriteBer(dest, (PhotonBerValue)gen->errorControlType));
    PHOTON_TRY(PhotonWriter_WriteBer(dest, gen->lastSequenceCounter));

    return gen->gen(gen->data, dest);
}

PhotonResult PhotonEncoder_EncodeReceiptPacket(PhotonReceiptPacketEnc* encoder, PhotonWriter* dest)
{
    return encodePacket(0x345c, encoder->errorControlType, encoder, receiptPacketGen, dest);
}

static PhotonResult counterAdjustmentPacketGen(void* data, PhotonWriter* dest)
{
    PhotonCounterAdjustmentPacketEnc* gen = (PhotonCounterAdjustmentPacketEnc*)data;
    PHOTON_TRY(PhotonWriter_WriteBer(dest, 1));
    PHOTON_TRY(PhotonWriter_WriteBer(dest, (PhotonBerValue)gen->streamType));
    PHOTON_TRY(PhotonWriter_WriteBer(dest, (PhotonBerValue)gen->errorControlType));
    PHOTON_TRY(PhotonWriter_WriteBer(dest, gen->sequenceCounter));

    return gen->gen(gen->data, dest);
}

PhotonResult PhotonEncoder_EncodeCounterAdjustmentPacket(PhotonCounterAdjustmentPacketEnc* encoder, PhotonWriter* dest)
{
    return encodePacket(0x645b, encoder->errorControlType, encoder, counterAdjustmentPacketGen, dest);
}

static PhotonResult exchangePacketGen(void* data, PhotonWriter* dest)
{
    PhotonExchangePacketEnc* gen = (PhotonExchangePacketEnc*)data;

    uint8_t* sizeDest = PhotonWriter_CurrentPtr(dest);
    PhotonWriter_Skip(dest, 3);

    PHOTON_TRY(PhotonWriter_WriteBer(dest, 1));
    PHOTON_TRY(PhotonWriter_WriteBer(dest, (PhotonBerValue)gen->streamType));
    PHOTON_TRY(PhotonWriter_WriteBer(dest, (PhotonBerValue)gen->errorControlType));
    PHOTON_TRY(PhotonWriter_WriteBer(dest, gen->windowSize));
    PHOTON_TRY(PhotonWriter_WriteBer(dest, gen->sequenceCounter));

    PHOTON_TRY(gen->gen(gen->data, dest));

    size_t dataSize = PhotonWriter_CurrentPtr(dest) - sizeDest;
    writeBer16Fixed(dataSize, sizeDest);

    return PhotonResult_Ok;
}

PhotonResult PhotonEncoder_EncodeExchangePacket(PhotonExchangePacketEnc* encoder, PhotonWriter* dest)
{
    return encodePacket(0x6c5a, encoder->errorControlType, encoder, exchangePacketGen, dest);
}
