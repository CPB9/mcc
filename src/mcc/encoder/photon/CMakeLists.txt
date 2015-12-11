include_directories(
    SYSTEM
    ${ASIO_INCLUDE_DIRS}
    ${BMCL_INCLUDE_DIRS}
    ${PHOTON_LIBRARIES}
)

mcc_add_library(mcc-encoder-photon-lib
    Service.h
    Service.cpp
    Device.h
    Device.cpp
    DeviceController.h
    DeviceController.cpp
    Photon.h
    Photon.cpp
)

set_target_properties(mcc-encoder-photon-lib
    PROPERTIES
    FOLDER "mcc/encoder"
)

target_link_libraries(mcc-encoder-photon-lib
    ${ASIO_LIBRARIES}
    ${PHOTON_LIBRARIES}
    mcc-encoder-core-lib
)

#mcc_add_executable(mcc-encoder-photon
#    main.cpp
#)

#set_target_properties(mcc-encoder-photon
#    PROPERTIES
#    FOLDER "bin"
#)

#target_link_libraries(mcc-encoder-photon
#    mcc-encoder-photon-lib
#)
