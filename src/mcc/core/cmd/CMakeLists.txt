include_directories(
    SYSTEM
)

mcc_add_library(mcc-core-cmd-lib
    Service.h
    Service.cpp
)

set_target_properties(mcc-core-cmd-lib
    PROPERTIES
    FOLDER "mcc/core"
)

target_link_libraries(mcc-core-cmd-lib
    mcc-misc-lib
    mcc-messages-lib
)

#mcc_add_executable(mcc-core-cmd
#    main.cpp
#)

#set_target_properties(mcc-core-cmd
#    PROPERTIES
#    FOLDER "bin"
#)

#target_link_libraries(mcc-core-cmd
#    mcc-core-cmd-lib
#)
