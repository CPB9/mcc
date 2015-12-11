mcc_add_library(mcc-core-decode-lib
    Common.h
    Sqlite3RegistryProvider.h
    Sqlite3RegistryProvider.cpp
    DecodeSourcesRegistryProvider.h
    DecodeSourcesRegistryProvider.cpp
    Sqlite3RegistryProviderSingleton.h
    Sqlite3RegistryProviderSingleton.cpp
    Registry.h
    Registry.cpp
    Variant.h
    Variant.cpp
)

include_directories(
    SYSTEM
    ${BMCL_INCLUDE_DIRS}
    ${PEGTL_INCLUDE_DIRS}
)

set_target_properties(mcc-core-decode-lib
    PROPERTIES
    FOLDER "mcc/core"
)

target_link_libraries(mcc-core-decode-lib
    bmcl
    Qt5::Sql
    mcc-misc-lib
)
