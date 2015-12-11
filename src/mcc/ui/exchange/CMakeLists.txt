include_directories(
    SYSTEM
)

mcc_add_library(mcc-ui-exchange-lib
    Service.h
    Service.cpp
)

set_target_properties(mcc-ui-exchange-lib
    PROPERTIES
    FOLDER "mcc/ui"
)

target_link_libraries(mcc-ui-exchange-lib
    Qt5::Core
    mcc-misc-lib
    mcc-messages-lib
)
