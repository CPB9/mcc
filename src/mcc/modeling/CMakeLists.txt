include_directories(
    SYSTEM
    ${ASIO_INCLUDE_DIRS}
)

mcc_add_library(mcc-modeling-lib
    2DMath.h
    2DMath.cpp
    Model.h
    FSM.h
    flying_device.h
    SimpleModel.h
    SimpleModel.cpp
)

set_target_properties(mcc-modeling-lib
    PROPERTIES
    FOLDER "mcc/modeling"
)

target_link_libraries(mcc-modeling-lib
    ${ASIO_LIBRARIES}
    mcc-misc-lib
    mcc-messages-lib
)

mcc_add_executable(mcc-modeling
    main.cpp
)

set_target_properties(mcc-modeling
    PROPERTIES
    FOLDER "bin"
)

target_link_libraries(mcc-modeling
    mcc-modeling-lib
)
