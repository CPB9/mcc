#include "photon/Exchange.h"
#include "photon/RingBuffer.h"

#include <stdbool.h>

void PhotonUavExchange_Init(PhotonUavExchange* self)
{
    self->cmdInCounter = 0;
    self->cmdOutCounter = 0;
    self->tmCounter = 0;
    PhotonRingBuf_Init(&self->ringBufIn, self->inBuffer, sizeof(self->inBuffer));
    PhotonRingBuf_Init(&self->ringBufOut, self->outBuffer, sizeof(self->outBuffer));
    self->addressPacketHandler = 0;
    self->addressPacketHandlerData = 0;
}

void PhotonUavExchange_SetAddressPacketHandler(PhotonUavExchange* self, void* data, PhotonAddressPacketHandler handler)
{
    self->addressPacketHandler  = handler;
    self->addressPacketHandlerData = data;
}

void PhotonUavExchange_SetExchangeHandler(PhotonUavExchange* self, void* data, PhotonExchangeHandler handler)
{
    self->exchangePacketHandler = handler;
    self->exchangePacketHandlerData = data;
}

void PhotonUavExchange_AcceptIncomingData(PhotonUavExchange* self, const void* src, size_t size)
{
    PhotonRingBuf_Write(&self->ringBufIn, src, size);
}

#define HEADER_SIZE 2

static bool skipSeparator(PhotonRingBuf* ringBuf, uint16_t separator)
{
    const uint8_t firstByte = (separator & 0xf0) >> 8;;
    const uint8_t secondByte = separator & 0x0f;
    while (true) {
        if (PhotonRingBuf_ReadableSize(ringBuf) < HEADER_SIZE) {
            return false;
        }
        if (firstByte != PhotonRingBuf_PeekUint8(ringBuf, 0)) {
            PhotonRingBuf_Skip(ringBuf, 1);
            continue;
        }
        if (secondByte != PhotonRingBuf_PeekUint8(ringBuf, 1)) {
            PhotonRingBuf_Skip(ringBuf, 2);
            continue;
        }
        PhotonRingBuf_Skip(ringBuf, 2);
        return true;
    }
    return false;
}

static PhotonResult peekHeader(PhotonReader* src, uint16_t* dest)
{
    if ((src->end - src->current) < 3) {
        return PhotonResult_InvalidSize;
    }

    const uint8_t* start = src->current;
    *dest = PhotonReader_ReadUint16Be(src);
    src->current = start;

    return PhotonResult_Ok;
}

static PhotonResult parseUavIncomingPacket(PhotonUavExchange* self)
{
    PhotonReader src;
    PhotonReader_Init(&src, self->temp, sizeof(self->temp));
    uint16_t header;
    PHOTON_TRY(peekHeader(&src, &header));

    switch (header) {
    case PHOTON_EXCHANGE_PACKET_HEADER: {
        PhotonExchangePacket exchangePacket;
        PHOTON_TRY(PhotonDecoder_DecodeExchangePacket(&src, &exchangePacket));

        if (exchangePacket.header.streamType != PhotonStreamType_Commands) {
            return PhotonResult_InvalidStreamType;
        }

        if (exchangePacket.sequenceCounter != self->cmdInCounter) {
            //TODO: сгенерировать пакет согласования счетчика
            return PhotonResult_InvalidSequenceCounter;
        }

        self->cmdInCounter++;

        PhotonAddressPacket addressPacket;
        PHOTON_TRY(PhotonDecoder_DecodeAddressPacket(&src, &addressPacket));
        PHOTON_TRY(self->addressPacketHandler(self->addressPacketHandlerData, &addressPacket));
        break;
    }
    case PHOTON_COUNTER_ADJUSTMENT_PACKET_HEADER: {
        PhotonCounterAdjustmentPacket packet;
        PHOTON_TRY(PhotonDecoder_DecodeCounterAdjustmentPacket(&src, &packet));
        if (packet.header.streamType != PhotonStreamType_Commands) {
            return PhotonResult_InvalidStreamType;
        }
        self->cmdInCounter = packet.sequenceCounter;
        break;
    }
    default:
        return PhotonResult_InvalidPacketHeader;
    }
    return PhotonResult_Ok;
}

static bool findPacketInRingBuf(PhotonRingBuf* ringBuf, uint16_t separator, uint8_t* dest)
{
    if (!skipSeparator(ringBuf, separator)) {
        return false;
    }

    size_t incomingSize = PhotonRingBuf_ReadableSize(ringBuf);
    if (incomingSize < (HEADER_SIZE + 1)) {
        return false;
    }

    size_t packetSize;
    uint8_t berHeader = PhotonRingBuf_PeekUint8(ringBuf, HEADER_SIZE);
    if (berHeader & 0x80) {
        size_t berSize = berHeader & 0x7f;
        if (berSize > 8) {
            //TODO: send error
            PhotonRingBuf_Skip(ringBuf, 3);
            return false;
        }
        PhotonBerValue dataSize;
        PhotonRingBuf_Peek(ringBuf, &dataSize, berSize, 3); // big endian?
        packetSize = 3 + berSize + dataSize;
    } else {
        packetSize = 3 + berHeader;
    }

    if (packetSize > PHOTON_MAX_PACKET_SIZE) {
        //TODO: send error
        return false;
    }

    if (incomingSize < packetSize) {
        return false;
    }

    PhotonRingBuf_Read(ringBuf, dest, packetSize);
    return true;
}

bool PhotonUavExchange_HandleIncomingPacket(PhotonUavExchange* self)
{
    if (!findPacketInRingBuf(&self->ringBufIn, 0x4f07, self->temp)) {
        return false;
    }

    PhotonResult rv = parseUavIncomingPacket(self);
    if (rv != PhotonResult_Ok) {
        //TODO: handle error
        return false;
    }

    return true;
}

PhotonResult PhotonUavExchange_SendGroupPacket(PhotonUavExchange* self, const PhotonGroupAddress* address, void* data, PhotonGenerator gen)
{

}

PhotonResult PhotonUavExchange_SendMulticastPacket(PhotonUavExchange* self, const PhotonMulticastAddress* address, void* data, PhotonGenerator gen)
{

}

PhotonResult PhotonUavExchange_SendNetworkPacket(PhotonUavExchange* self, const PhotonNetworkAddress* address, void* data, PhotonGenerator gen)
{

}

/*------------------*/

void PhotonGcExchange_Init(PhotonGcExchange* self)
{
    self->cmdCounter = 0;
    self->tmCounter = 0;
    PhotonRingBuf_Init(&self->ringBufIn, self->inBuffer, sizeof(self->inBuffer));
    PhotonRingBuf_Init(&self->ringBufOut, self->outBuffer, sizeof(self->outBuffer));
    PhotonRingBuf_Init(&self->tmRingBuf, self->tmBuffer, sizeof(self->tmBuffer));
    self->readyToSend = true;
}

void PhotonGcExchange_SetExchangeHandler(PhotonGcExchange* self, void* data, PhotonExchangeHandler handler)
{
    self->exchangePacketHandler = handler;
    self->exchangePacketHandlerData = data;
}

void PhotonGcExchange_SetCommandResultHandler(PhotonGcExchange* self, void* data, PhotonCommandResultHandler handler)
{
    self->commandResultHandler = handler;
    self->commandResultHandlerData = data;
}

void PhotonGcExchange_SetTmEventHandler(PhotonGcExchange* self, void* data, PhotonTmEventMessageHandler handler)
{
    self->tmEventHandler = handler;
    self->tmEventHandlerData = data;
}

void PhotonGcExchange_SetTmStatusHandler(PhotonGcExchange* self, void* data, PhotonTmStatusMessageHandler handler)
{
    self->tmStatusHandler = handler;
    self->tmStatusHandlerData = data;
}

void PhotonGcExchange_AcceptIncomingData(PhotonGcExchange* self, const void* src, size_t size)
{
    PhotonRingBuf_Write(&self->ringBufIn, src, size);
}

static PhotonResult parseGcIncomingPacket(PhotonGcExchange* self)
{
    PhotonReader src;
    PhotonReader_Init(&src, self->temp, sizeof(self->temp));
    uint16_t header;
    PHOTON_TRY(peekHeader(&src, &header));

    switch (header) {
    case PHOTON_EXCHANGE_PACKET_HEADER: {
        PhotonExchangePacket exchangePacket;
        PHOTON_TRY(PhotonDecoder_DecodeExchangePacket(&src, &exchangePacket));

        if (exchangePacket.header.streamType != PhotonStreamType_Telemetry) {
            return PhotonResult_InvalidStreamType;
        }

        if (exchangePacket.sequenceCounter != self->tmCounter) {
            //TODO: сообщить о пропавшей тм
        }

        self->tmCounter++;

        PhotonAddressPacket addressPacket;
        PHOTON_TRY(PhotonDecoder_DecodeAddressPacket(&src, &addressPacket));
        PhotonRingBuf_Write(&self->tmRingBuf, &addressPacket.data, PhotonReader_ReadableSize(&addressPacket.data));
        while (findPacketInRingBuf(&self->ringBufIn, 0x043d, self->tmTemp)) {
            PhotonReader_Init(&src, self->tmTemp, sizeof(self->tmTemp));
            uint16_t header;
            PHOTON_TRY(peekHeader(&src, &header));
            if (header == 0x0c78) {
                PhotonTmEventMessage msg;
                if (PhotonDecoder_DecodeTmEventMessage(&src, &msg) != PhotonResult_Ok) {
                    continue;
                } else {
                    PHOTON_TRY(self->tmEventHandler(self->tmEventHandlerData, &msg));
                }
            } else if (header == 0x1c72) {
                PhotonTmStatusMessage msg;
                if (PhotonDecoder_DecodeTmStatusMessage(&src, &msg) != PhotonResult_Ok) {
                    continue;
                } else {
                    PHOTON_TRY(self->tmStatusHandler(self->tmStatusHandlerData, &msg));
                }
            }
        }
        break;
    }
    case PHOTON_RECEIPT_PACKET_HEADER: {
        PhotonReceiptPacket packet;
        PHOTON_TRY(PhotonDecoder_DecodeReceiptPacket(&src, &packet));
        if (packet.header.streamType != PhotonStreamType_Commands) {
            return PhotonResult_InvalidStreamType;
        }
        self->readyToSend = packet.lastSequenceCounter == self->cmdCounter;
        break;
    }
    default:
        return PhotonResult_InvalidPacketHeader;
    }
    return PhotonResult_Ok;
}

bool PhotonGcExchange_HandleIncomingPacket(PhotonGcExchange* self)
{
    if (!findPacketInRingBuf(&self->ringBufIn, 0xacac, self->temp)) {
        return false;
    }

    PhotonResult rv = parseGcIncomingPacket(self);
    if (rv != PhotonResult_Ok) {
        //TODO: handle error
        return false;
    }

    return true;
}

PhotonResult PhotonGcExchange_SendGroupPacket(PhotonGcExchange* self, const PhotonGroupAddress* address, void* data, PhotonGenerator gen)
{

}

PhotonResult PhotonGcExchange_SendMulticastPacket(PhotonGcExchange* self, const PhotonMulticastAddress* address, void* data, PhotonGenerator gen)
{

}

PhotonResult PhotonGcExchange_SendNetworkPacket(PhotonGcExchange* self, const PhotonNetworkAddress* address, void* data, PhotonGenerator gen)
{

}
