include_directories(
    SYSTEM
    ${BMCL_INCLUDE_DIRS}
)

mcc_add_library(mcc-misc-lib
    Channel.h
    ChannelConsumer.h
    Cmd.h
    CommonMath.h
    Crc.cpp
    Crc.h
    Device.h
    Either.h
    Firmware.h
    Helpers.h
    Net.h
    Net.cpp
    NetStatistics.h
    NetVariant.h
    NetVariant.cpp
    Option.h
    ProcessIni.h
    ProcessIni.cpp
    Protocol.h
    Result.h
    Route.h
    Runnable.h
    SharedVar.h
    TaskPool.h
    TimeUtils.h
    TmParam.h
    tm_utils.h
    tm_utils.cpp
    TraitSort.h
    TraitSort.cpp
)

set_property(
    TARGET
    mcc-misc-lib
    PROPERTY
    INTERFACE_INCLUDE_DIRECTORIES
    ${BMCL_INCLUDE_DIRS}
)

set_target_properties(mcc-misc-lib
    PROPERTIES
    FOLDER "mcc"
)

target_link_libraries(mcc-misc-lib
    ${PROTOBUF_LIBRARIES}
    Qt5::Core
    bmcl
)
