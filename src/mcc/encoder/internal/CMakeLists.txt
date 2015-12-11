include_directories(
    SYSTEM
    ${ASIO_INCLUDE_DIRS}
    ${BMCL_INCLUDE_DIRS}
)

mcc_add_library(mcc-encoder-internal-lib
    Service.h
    Service.cpp
    Device.h
    Device.cpp
    DeviceReader.h
    DeviceReader.cpp
    DeviceCommander.h
    DeviceCommander.cpp
    Internal.h
    Internal.cpp
)

set_target_properties(mcc-encoder-internal-lib
    PROPERTIES
    FOLDER "mcc/encoder"
)

target_link_libraries(mcc-encoder-internal-lib
    ${ASIO_LIBRARIES}
    mcc-encoder-core-lib
)

#mcc_add_executable(mcc-encoder-internal
#    main.cpp
#)

#set_target_properties(mcc-encoder-internal
#    PROPERTIES
#    FOLDER "bin"
#)

#target_link_libraries(mcc-encoder-internal
#    mcc-encoder-internal-lib
#)
