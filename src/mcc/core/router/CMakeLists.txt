include_directories(
    SYSTEM
)

mcc_add_library(mcc-core-router-lib
    Service.h
    Service.cpp
)

set_target_properties(mcc-core-router-lib
    PROPERTIES
    FOLDER "mcc/core"
)

target_link_libraries(mcc-core-router-lib
    mcc-misc-lib
    mcc-messages-lib
)

#mcc_add_executable(mcc-core-router
#    main.cpp
#)

#set_target_properties(mcc-core-router
#    PROPERTIES
#    FOLDER "bin"
#)

#target_link_libraries(mcc-core-router
#    mcc-core-router-lib
#)
