include_directories(
    SYSTEM
    ${BMCL_INCLUDE_DIRS}
)

mcc_add_library(mcc-ui-s57-lib
    Parser.cpp
    Parser.h
    Directory.h
    ParseTree.h
    RecordDescriptor.h
    Ptr.h
    Fields.h
    Record.h
)

target_link_libraries(mcc-ui-s57-lib
    bmcl
)

set_target_properties(mcc-ui-s57-lib
    PROPERTIES
    FOLDER "mcc/ui/s57"
)
