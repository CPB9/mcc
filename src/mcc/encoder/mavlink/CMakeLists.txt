include_directories(
    SYSTEM
    ${ASIO_INCLUDE_DIRS}
    ${MAVLINK_INCLUDE_DIRS}
    ${BMCL_INCLUDE_DIRS}
)

mcc_add_library(mcc-encoder-mavlink-lib
    Service.h
    Service.cpp
    Device.h
    Device.cpp
    DeviceController.h
    DeviceController.cpp
    Mavlink.h
    Mavlink.cpp
    mavlink_utils.h
    mavlink_utils.cpp
    RouteController.h
    RouteController.cpp
)

set_target_properties(mcc-encoder-mavlink-lib
    PROPERTIES
    FOLDER "mcc/encoder"
)

target_link_libraries(mcc-encoder-mavlink-lib
    ${ASIO_LIBRARIES}
    mcc-encoder-core-lib
    mcc-core-decode-lib
    mcc-misc-lib
    mcc-modeling-lib
)

#mcc_add_executable(mcc-encoder-mavlink
#    main.cpp
#)

#set_target_properties(mcc-encoder-mavlink
#    PROPERTIES
#    FOLDER "bin"
#)

#target_link_libraries(mcc-encoder-mavlink
#    mcc-encoder-mavlink-lib
#)
