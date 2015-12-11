#include "photon/Parser.h"

static PhotonResult peekHeader(PhotonReader* src, uint16_t* dest)
{
    if ((src->end - src->current) < 3) {
        return PhotonResult_InvalidSize;
    }

    //TODO: сделать функцию peekUint16Be
    const uint8_t* start = src->current;
    *dest = PhotonReader_ReadUint16Be(src);
    src->current = start;

    return PhotonResult_Ok;
}

PhotonResult PhotonParser_ParseMultiplexStream(PhotonParser* self, PhotonReader* src)
{
    uint16_t header;
    PHOTON_TRY(peekHeader(src, &header));

    switch (header) {
    case PHOTON_EXCHANGE_PACKET_HEADER: {
        PhotonExchangePacket packet;
        PHOTON_TRY(PhotonDecoder_DecodeExchangePacket(src, &packet));
        PHOTON_TRY(self->handleExchangePacket(self->data, &packet));
        return PhotonParser_ParseExchangeStream(self, &packet.data);
    }
    case PHOTON_COUNTER_ADJUSTMENT_PACKET_HEADER: {
        PhotonCounterAdjustmentPacket packet;
        PHOTON_TRY(PhotonDecoder_DecodeCounterAdjustmentPacket(src, &packet));
        return self->handleCounterAdjustmentPacket(self->data, &packet);
    }
    case PHOTON_RECEIPT_PACKET_HEADER: {
        PhotonReceiptPacket packet;
        PHOTON_TRY(PhotonDecoder_DecodeReceiptPacket(src, &packet));
        return self->handleReceiptPacket(self->data, &packet);
    }
    default:
        return PhotonResult_InvalidPacketHeader;
    }
}

PhotonResult PhotonParser_ParseExchangeStream(PhotonParser* self, PhotonReader* src)
{
    PhotonAddressPacket packet;
    while (!PhotonReader_IsAtEnd(src)) {
        PHOTON_TRY(PhotonDecoder_DecodeAddressPacket(src, &packet));
        PHOTON_TRY(self->handleAddressPacket(self->data, &packet));
        PHOTON_TRY(PhotonParser_ParseApplicationStream(self, &packet.data));
    }
    return PhotonResult_Ok;
}

PhotonResult PhotonParser_ParseApplicationStream(PhotonParser* self, PhotonReader* src)
{
    switch (self->type) {
    case PhotonStreamType_Commands:
        return PhotonParser_ParseCommandStream(self, src);
    case PhotonStreamType_Telemetry:
        return PhotonParser_ParseTelemetryStream(self, src);
    default:
        return PhotonResult_InvalidStreamType;
    }
}

PhotonResult PhotonParser_ParseCommandStream(PhotonParser* self, PhotonReader* src)
{
    PhotonCommandMessage msg;
    while (!PhotonReader_IsAtEnd(src)) {
        PHOTON_TRY(PhotonDecoder_DecodeCommandMessage(src, &msg));
        PHOTON_TRY(self->handleCommandMessage(self->data, &msg));
    }
    return PhotonResult_Ok;
}

PhotonResult PhotonParser_ParseTelemetryStream(PhotonParser* self, PhotonReader* src)
{

}
