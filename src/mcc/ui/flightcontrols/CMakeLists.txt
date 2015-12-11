qt5_wrap_ui(FLIGHT_CONTROLS_UIS
            DeviceStateWidget.ui
            DeviceSettingsWidget.ui
            TestWindow.ui
            )

SET(SOURCE_FILES
    PrimaryFlightDisplay.h
    PrimaryFlightDisplay.cpp
    TmParameterWidget.h
    TmParameterWidget.cpp
    SignalBar.h
    SignalBar.cpp
    DeviceStateWidget.h
    DeviceStateWidget.cpp
    DevicesListWidget.h
    DevicesListWidget.cpp
    DeviceSettingsWidget.h
    DeviceSettingsWidget.cpp
    SliderCheckBox.h
    SliderCheckBox.cpp
    NetStatisticsWidget.h
)

mcc_add_library(mcc-ui-flightcontrols-lib
    ${SOURCE_FILES}
    ${FLIGHT_CONTROLS_UIS}
)

target_link_libraries(mcc-ui-flightcontrols-lib
    Qt5::Core
    Qt5::Gui
    Qt5::Svg
    Qt5::Widgets
    Qt5::OpenGL
    Qt5::Network
    Qwt
)

target_link_libraries(mcc-ui-flightcontrols-lib
    mcc-ui-core-lib
)

set_target_properties(mcc-ui-flightcontrols-lib
    PROPERTIES
    FOLDER "mcc/ui"
)

#mcc_add_executable(mcc-ui-flightcontrols
#    main.cpp
#    TestWindow.h
#    TestWindow.cpp)

#target_link_libraries(mcc-ui-flightcontrols
#    mcc-ui-flightcontrols-lib
#)


#set_target_properties(mcc-ui-flightcontrols
#    PROPERTIES
#    FOLDER "bin"
#)