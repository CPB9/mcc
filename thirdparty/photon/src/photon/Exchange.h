#ifndef __PHOTON_EXCHANGE_H__
#define __PHOTON_EXCHANGE_H__

#include "photon/RingBuffer.h"
#include "photon/Decoder.h"
#include "photon/Encoder.h"

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#define PHOTON_MAX_PACKET_SIZE 1024
#define PHOTON_MAX_INCOMING_PACKETS 10
#define PHOTON_MAX_OUTGOING_PACKETS 10
#define PHOTON_MAX_TM_MESSAGE_SIZE 512
#define PHOTON_MAX_TM_MESSAGES 10

typedef PhotonResult (*PhotonAddressPacketHandler)(void*, PhotonAddressPacket*);
typedef PhotonResult (*PhotonExchangeHandler)(void*, PhotonRingBuf*);
typedef PhotonResult (*PhotonCommandResultHandler)(void*, PhotonCommandResult*);
typedef PhotonResult (*PhotonTmEventMessageHandler)(void*, PhotonTmEventMessage*);
typedef PhotonResult (*PhotonTmStatusMessageHandler)(void*, PhotonTmStatusMessage*);

typedef struct {
    uint16_t cmdInCounter;
    uint16_t cmdOutCounter;
    uint16_t tmCounter;
    PhotonRingBuf ringBufIn;
    PhotonRingBuf ringBufOut;
    uint32_t pad1;
    uint8_t inBuffer[PHOTON_MAX_PACKET_SIZE * PHOTON_MAX_INCOMING_PACKETS];
    uint32_t pad2;
    uint8_t outBuffer[PHOTON_MAX_PACKET_SIZE * PHOTON_MAX_OUTGOING_PACKETS];
    uint8_t temp[PHOTON_MAX_PACKET_SIZE];

    PhotonAddressPacketHandler addressPacketHandler;
    void* addressPacketHandlerData;

    PhotonExchangeHandler exchangePacketHandler;
    void* exchangePacketHandlerData;
} PhotonUavExchange;

void PhotonUavExchange_Init(PhotonUavExchange* self);
void PhotonUavExchange_SetAddressPacketHandler(PhotonUavExchange* self, void* data, PhotonAddressPacketHandler handler);
void PhotonUavExchange_SetExchangeHandler(PhotonUavExchange* self, void* data, PhotonExchangeHandler handler);

void PhotonUavExchange_AcceptIncomingData(PhotonUavExchange* self, const void* src, size_t size);
bool PhotonUavExchange_HandleIncomingPacket(PhotonUavExchange* self);

PhotonResult PhotonUavExchange_SendMulticastPacket(PhotonUavExchange* self, const PhotonMulticastAddress* address,
                                                   void* data, PhotonGenerator gen);
PhotonResult PhotonUavExchange_SendGroupPacket(PhotonUavExchange* self, const PhotonGroupAddress* address, void* data,
                                               PhotonGenerator gen);
PhotonResult PhotonUavExchange_SendNetworkPacket(PhotonUavExchange* self, const PhotonNetworkAddress* address,
                                                 void* data, PhotonGenerator gen);

// 1) обменка состоит из 2 кольцевых буферов пакетов и 1 буфера телеметрической информации
// 2) при при отправке пакеты сохраняются в кольцевом буфере, после этого дергается функция указатель по которой
// пользователь должен забрать данные или их часть из кольцевого буфера
// 3) при приеме пакета пользователь дергает функцию для принятия части данных от борта и складывания их в кольцевой буфера
// данные могут приходить кусками, и после каждого куска пользователь должен дергать функцию поиска пакета в кольцевом буфере
// 4) входящие пакеты для наземки - телеметрия, отправляемая кусками своим собвственным потоком
// 5) после принятия пакета телеметрии, телеметрия складывается в буфер телеметрии и производится поиск тм кадров, при
// нахождении которых дергаются функции указатели
// 6) пакеты квитанций обрабатываются автоматически
// 7) при нахождении пакета ответа на команду дергается функция указатель

typedef struct {
    uint16_t cmdCounter;
    uint16_t tmCounter;
    PhotonRingBuf ringBufIn;
    PhotonRingBuf ringBufOut;
    PhotonRingBuf tmRingBuf;
    uint8_t inBuffer[PHOTON_MAX_PACKET_SIZE * PHOTON_MAX_INCOMING_PACKETS];
    uint8_t outBuffer[PHOTON_MAX_PACKET_SIZE * PHOTON_MAX_OUTGOING_PACKETS];
    uint8_t tmBuffer[PHOTON_MAX_TM_MESSAGE_SIZE * PHOTON_MAX_TM_MESSAGES];
    uint8_t temp[PHOTON_MAX_PACKET_SIZE];
    uint8_t tmTemp[PHOTON_MAX_TM_MESSAGE_SIZE];
    bool readyToSend;

    PhotonExchangeHandler exchangePacketHandler;
    void* exchangePacketHandlerData;

    PhotonTmEventMessageHandler tmEventHandler;
    void* tmEventHandlerData;

    PhotonTmStatusMessageHandler tmStatusHandler;
    void* tmStatusHandlerData;

    PhotonCommandResultHandler commandResultHandler;
    void* commandResultHandlerData;
} PhotonGcExchange;

void PhotonGcExchange_Init(PhotonGcExchange* self);

// принять часть пакета от борта
void PhotonGcExchange_AcceptIncomingData(PhotonGcExchange* self, const void* src, size_t size);

// попытаться выделить и обработать пакет в кольцевом буфере входящих данных
bool PhotonGcExchange_HandleIncomingPacket(PhotonGcExchange* self);

// установить хендлер обмена, который дергается для отправки данных по сети после формирования пакета
void PhotonGcExchange_SetExchangeHandler(PhotonGcExchange* self, void* data, PhotonExchangeHandler handler);

// установить хендлер статусных сообщений
void PhotonGcExchange_SetTmStatusHandler(PhotonGcExchange* self, void* data, PhotonTmStatusMessageHandler handler);

// установить хендлер событийных сообщений
void PhotonGcExchange_SetTmEventHandler(PhotonGcExchange* self, void* data, PhotonTmEventMessageHandler handler);

// установить хендлер ответов на команды
void PhotonGcExchange_SetCommandResultHandler(PhotonGcExchange* self, void* data, PhotonCommandResultHandler handler);

// отправка пакетов
PhotonResult PhotonGcExchange_SendMulticastPacket(PhotonGcExchange* self, const PhotonMulticastAddress* address,
                                                  void* data, PhotonGenerator gen);
PhotonResult PhotonGcExchange_SendGroupPacket(PhotonGcExchange* self, const PhotonGroupAddress* address, void* data,
                                              PhotonGenerator gen);
PhotonResult PhotonGcExchange_SendNetworkPacket(PhotonGcExchange* self, const PhotonNetworkAddress* address, void* data,
                                                PhotonGenerator gen);

#ifdef __cplusplus
}
#endif

#endif
