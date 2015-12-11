include_directories(
    SYSTEM
    ${NMEA_INCLUDE_DIRS}
)


mcc_add_library(mcc-ui-core-lib
    Settings.h
    Settings.cpp
    Structs.h
    Structs.cpp
    FlyingDevice.h
    FlyingDevice.cpp
    DeviceManager.h
    DeviceManager.cpp
    Route.cpp
    Route.h
    CoordinatePrinter.h
    CoordinatePrinter.cpp
    CoordinateEditor.h
    CoordinateEditor.cpp
    GlobalCoordinatePrinter.h
    GlobalCoordinatePrinter.cpp
    LatLonEdit.h
    LatLonEdit.cpp
    WaypointEdit.h
    WaypointEdit.cpp
    WaypointFlagEdit.h
    WaypointFlagEdit.cpp
    DevicesGroup.h
    PositionProvider.h
)

target_link_libraries(mcc-ui-core-lib
    Qt5::Core
    Qt5::Gui
    Qt5::Widgets
    Qt5::SerialPort
    mcc-misc-lib
    nmealib
)

set_target_properties(mcc-ui-core-lib
    PROPERTIES
    FOLDER "mcc/ui"
)
