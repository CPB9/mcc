include_directories(
    SYSTEM
)

mcc_add_library(mcc-core-manager-lib
    Service.h
    Service.cpp
)

set_target_properties(mcc-core-manager-lib
    PROPERTIES
    FOLDER "mcc/core"
)

target_link_libraries(mcc-core-manager-lib
    mcc-messages-lib
    mcc-core-tm-lib
    mcc-core-db-lib
    mcc-core-router-lib
    mcc-core-cmd-lib
    mcc-core-decode-lib
    mcc-encoder-internal-lib
    mcc-encoder-mavlink-lib
    mcc-encoder-photon-lib
    mcc-modeling-lib
)

#mcc_add_executable(mcc-core-manager
#    main.cpp
#)

#set_target_properties(mcc-core-manager
#    PROPERTIES
#    FOLDER "bin"
#)

#target_link_libraries(mcc-core-manager
#    mcc-core-manager-lib
#)
