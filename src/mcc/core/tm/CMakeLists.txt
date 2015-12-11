include_directories(
    SYSTEM
)

mcc_add_library(mcc-core-tm-lib
    Service.h
    Service.cpp
)

set_target_properties(mcc-core-tm-lib
    PROPERTIES
    FOLDER "mcc/core"
)

target_link_libraries(mcc-core-tm-lib
    mcc-misc-lib
    mcc-messages-lib
)

#mcc_add_executable(mcc-core-tm
#    main.cpp
#)

#set_target_properties(mcc-core-tm
#    PROPERTIES
#    FOLDER "bin"
#)

#target_link_libraries(mcc-core-tm
#    mcc-core-tm-lib
#)
