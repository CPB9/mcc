include_directories(
    SYSTEM
    ${ASIO_INCLUDE_DIRS}
    ${BMCL_INCLUDE_DIRS}
)

mcc_add_library(mcc-encoder-core-lib
    Channel.h
    Channel.cpp
    ChannelCom.h
    ChannelUdp.h
    ChannelTcp.h
    ChannelManager.h
    ChannelManager.cpp
    Device.h
    Device.cpp
    DeviceTask.h
    ExchangeError.h
    Exchanger.h
    ExchangeHelper.h
    Packet.h
    Requester.h
    Service.h
    Service.cpp
)

set_target_properties(mcc-encoder-core-lib
    PROPERTIES
    FOLDER "mcc/encoder"
)

target_link_libraries(mcc-encoder-core-lib
    ${ASIO_LIBRARIES}
    bmcl
    mcc-misc-lib
    mcc-messages-lib
)
