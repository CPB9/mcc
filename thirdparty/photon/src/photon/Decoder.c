#include "photon/Decoder.h"

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>

static PhotonResult decodeDataHeader(PhotonReader* src, PhotonDataHeader* dest, uint16_t expectedCode)
{
    if (PhotonReader_ReadableSize(src) < 3) {
        return PhotonResult_InvalidDataSize;
    }

    dest->code = PhotonReader_ReadUint16Be(src);
    if (dest->code != expectedCode) {
        return PhotonResult_InvalidDataHeader;
    }

    PHOTON_TRY(PhotonReader_ReadBer(src, &dest->length));

    return PhotonResult_Ok;
}

static PhotonResult sliceData(PhotonReader* src, size_t length, PhotonReader* dest)
{
    if (length < PhotonReader_ReadableSize(src)) {
        return PhotonResult_InvalidDataSize;
    }

    PhotonReader_Slice(src, length, dest);

    return PhotonResult_Ok;
}

PhotonResult PhotonDecoder_DecodeCommandMessage(PhotonReader* src, PhotonCommandMessage* dest)
{
    PHOTON_TRY(decodeDataHeader(src, &dest->header, PHOTON_COMMAND_MESSAGE_HEADER));

    PHOTON_TRY(sliceData(src, dest->header.length, &dest->commands));

    return PhotonResult_Ok;
}

PhotonResult PhotonDecoder_DecodeCommandResult(PhotonReader* src, PhotonCommandResult* dest)
{
    PHOTON_TRY(decodeDataHeader(src, &dest->header, PHOTON_COMMAND_RESULT_HEADER));

    PHOTON_TRY(sliceData(src, dest->header.length, &dest->resultData));

    return PhotonResult_Ok;
}

PhotonResult PhotonDecoder_DecodeTmStatusMessage(PhotonReader* src, PhotonTmStatusMessage* dest)
{
    PHOTON_TRY(decodeDataHeader(src, &dest->header, PHOTON_TM_STATUS_MESSAGE_HEADER));

    PHOTON_TRY(PhotonReader_ReadBer(src, &dest->segmentNumber));
    PHOTON_TRY(PhotonReader_ReadBer(src, &dest->maxSegmentNumber));
    PHOTON_TRY(PhotonReader_ReadBer(src, &dest->componentNumber));
    PHOTON_TRY(PhotonReader_ReadBer(src, &dest->messageNumber));

    PHOTON_TRY(sliceData(src, dest->header.length, &dest->parameters));

    return PhotonResult_Ok;
}

PhotonResult PhotonDecoder_DecodeTmEventMessage(PhotonReader* src, PhotonTmEventMessage* dest)
{
    PHOTON_TRY(decodeDataHeader(src, &dest->header, PHOTON_TM_EVENT_MESSAGE_HEADER));

    PHOTON_TRY(PhotonReader_ReadBer(src, &dest->componentNumber));
    PHOTON_TRY(PhotonReader_ReadBer(src, &dest->messageNumber));
    PHOTON_TRY(PhotonReader_ReadBer(src, &dest->eventNumber));
    PHOTON_TRY(PhotonReader_ReadBer(src, &dest->timestamp));

    PHOTON_TRY(sliceData(src, dest->header.length, &dest->parameters));

    return PhotonResult_Ok;
}

PhotonResult PhotonDecoder_DecodeAddressPacket(PhotonReader* src, PhotonAddressPacket* dest)
{
    PHOTON_TRY(decodeDataHeader(src, &dest->header, PHOTON_ADDRESS_PACKET_HEADER));

    PhotonBerValue addressType;
    PHOTON_TRY(PhotonReader_ReadBer(src, &addressType));
    PHOTON_TRY(PhotonReader_ReadBer(src, &dest->srcAddress));
    PHOTON_TRY(PhotonReader_ReadBer(src, &dest->srcComponentNumber));
    PHOTON_TRY(PhotonReader_ReadBer(src, &dest->destComponentNumber));

    switch (addressType) {
    case 3:
        dest->addressType = PhotonAddressType_Broadcast;
        break;
    case 4:
        dest->addressType = PhotonAddressType_NetworkAddress;
        PHOTON_TRY(PhotonReader_ReadBer(src, &dest->destAddress));
        break;
    case 6:
        dest->addressType = PhotonAddressType_GroupAddress;
        PHOTON_TRY(PhotonReader_ReadBer(src, &dest->destAddress));
        PHOTON_TRY(PhotonReader_ReadBer(src, &dest->srcGroup));
        PHOTON_TRY(PhotonReader_ReadBer(src, &dest->destGroup));
        break;
    default:
        return PhotonResult_InvalidAddressType;
    }

    PHOTON_TRY(PhotonReader_ReadBer(src, &dest->timestampType));
    PHOTON_TRY(PhotonReader_ReadBer(src, &dest->timestamp));

    PHOTON_TRY(sliceData(src, dest->header.length, &dest->data));

    return PhotonResult_Ok;
}

static PhotonResult decodeExchangePacketHeader(PhotonReader* src, PhotonPacketHeader* dest, const uint8_t** end, uint16_t expectedCode)
{
    const uint8_t* packetStart = PhotonReader_CurrentPtr(src);

     if (PhotonReader_ReadableSize(src) < 5) {
        return PhotonResult_InvalidDataSize;
    }

    dest->code = PhotonReader_ReadUint16Be(src);
    if (dest->code != expectedCode) {
        return PhotonResult_InvalidDataHeader;
    }

    PHOTON_TRY(PhotonReader_ReadBer(src, &dest->length));
    PHOTON_TRY(PhotonReader_ReadBer(src, &dest->reserved));

    if (dest->reserved != 1) {
        return PhotonResult_InvalidReservedFieldValue;
    }

    PhotonBerValue streamType;
    PHOTON_TRY(PhotonReader_ReadBer(src, &streamType));

    switch (streamType) {
    case 0:
        dest->streamType = PhotonStreamType_Commands;
        break;
    case 1:
        dest->streamType = PhotonStreamType_Telemetry;
        break;
    default:
        return PhotonResult_InvalidStreamType;
    };

    PhotonBerValue errorControlType;
    PHOTON_TRY(PhotonReader_ReadBer(src, &errorControlType));

    if (PhotonReader_ReadableSize(src) < dest->length) {
        return PhotonResult_InvalidDataSize;
    }

    const uint8_t* packetEnd = PhotonReader_CurrentPtr(src) + dest->length;
    unsigned checkedSize = packetEnd - packetStart;

    switch (errorControlType) {
    case 1:
        dest->errorControlType = PhotonErrorControlType_Crc16;
        packetEnd -= 2;
        uint16_t checksumCrc16;
        memcpy(&checksumCrc16, packetEnd, 2);
        (void)checkedSize;
        // TODO: check crc16
        break;
    case 2:
        dest->errorControlType = PhotonErrorControlType_ReedSolomon;
        packetEnd -= 4;
        uint32_t checksumRs;
        memcpy(&checksumRs, packetEnd, 4);
        (void)checkedSize;
        // TODO: check rs
        break;
    default:
        return PhotonResult_InvalidErrorControlType;
    };

    *end = packetEnd;

    return PhotonResult_Ok;
}

PhotonResult PhotonDecoder_DecodeExchangePacket(PhotonReader* src, PhotonExchangePacket* dest)
{
    const uint8_t* packetEnd;
    PHOTON_TRY(decodeExchangePacketHeader(src, &dest->header, &packetEnd, PHOTON_EXCHANGE_PACKET_HEADER));

    PHOTON_TRY(PhotonReader_ReadBer(src, &dest->windowSize));

    if (dest->windowSize > (UINT16_MAX / 2)) {
        return PhotonResult_InvalidWindowSize;
    }

    PHOTON_TRY(PhotonReader_ReadBer(src, &dest->sequenceCounter));

    if (dest->sequenceCounter > UINT16_MAX) {
        return PhotonResult_InvalidSequenceCounter;
    }

    PhotonReader_Slice(src, PhotonReader_CurrentPtr(src) - packetEnd, &dest->data);

    // TODO: skip CS

    return PhotonResult_Ok;
}

PhotonResult PhotonDecoder_DecodeCounterAdjustmentPacket(PhotonReader* src, PhotonCounterAdjustmentPacket* dest)
{
    const uint8_t* packetEnd;
    PHOTON_TRY(decodeExchangePacketHeader(src, &dest->header, &packetEnd, PHOTON_COUNTER_ADJUSTMENT_PACKET_HEADER));

    PHOTON_TRY(PhotonReader_ReadBer(src, &dest->sequenceCounter));

    // TODO: skip CS

    return PhotonResult_Ok;
}

PhotonResult PhotonDecoder_DecodeReceiptPacket(PhotonReader* src, PhotonReceiptPacket* dest)
{
    const uint8_t* packetEnd;
    PHOTON_TRY(decodeExchangePacketHeader(src, &dest->header, &packetEnd, PHOTON_RECEIPT_PACKET_HEADER));

    if (dest->header.streamType != PhotonStreamType_Commands) {
        return PhotonResult_InvalidStreamType;
    }

    PHOTON_TRY(PhotonReader_ReadBer(src, &dest->lastSequenceCounter));

    // TODO: skip CS

    return PhotonResult_Ok;
}
