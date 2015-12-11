qt5_add_resources(MCC_CORE_DB_RESOURCES Db.qrc)

include_directories(
    SYSTEM
)

set (QUERIES
    queries/TmParamList.h
    queries/Cmd.h
    queries/CmdState.h
    queries/DeviceActionLog.h
    queries/DeviceRegister_Request.cpp
    queries/DeviceUnRegister_Request.cpp
    queries/DeviceList_Request.h
    queries/DeviceDescription_Request.h
    queries/DeviceUpdate_Request.cpp
    queries/FirmwareDescription_Request.cpp
    queries/FirmwareList_Request.h
    queries/FirmwareRegister_Request.cpp
    queries/ProtocolList_Request.h
    queries/ProtocolDescription_Request.h
    queries/ProtocolForDevice_Request.h
    queries/ProtocolForDeviceRegister_Request.h
)
source_group("Queries" FILES ${QUERIES})

set (DBRESOURCES
    db/SchemaRsceStyle.sql
    db/Schema.sql
    db/traits/CoreBase.yaml
    db/traits/CoreBase.json
    db/traits/Devices.yaml
    db/traits/Devices.json
    db/traits/DeviceKinds.yaml
    db/traits/DeviceKinds.json
    db/traits/MavlinkCommon.yaml
    db/traits/MavlinkCommon.json
    db/traits/Model.yaml
    db/traits/Model.json
    db/traits/Protocols.yaml
    db/traits/Protocols.json
    db/traits/Services.yaml
    db/traits/Services.json
)
source_group("Resources" FILES ${DBRESOURCES})

mcc_add_library(mcc-core-db-lib
    DbHandle.h
    Service.h
    Service.cpp
    FirmwareLoader.h
    FirmwareLoader.cpp
    Sql.h
    ${QUERIES}
    ${DBRESOURCES}
    ${MCC_CORE_DB_RESOURCES}
)

set_target_properties(mcc-core-db-lib
    PROPERTIES
    FOLDER "mcc/core"
)

target_link_libraries(mcc-core-db-lib
    Qt5::Core
    Qt5::Sql
    mcc-misc-lib
    mcc-messages-lib
    mcc-core-decode-lib
)

#mcc_add_executable(mcc-core-db
#    main.cpp
#    ${MCC_CORE_DB_RESOURCES}
#)

#set_target_properties(mcc-core-db
#    PROPERTIES
#    FOLDER "bin"
#)

#target_link_libraries(mcc-core-db
#    mcc-core-db-lib
#)
