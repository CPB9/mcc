include_directories(
    SYSTEM
    ${BMCL_INCLUDE_DIRS}
    ${CURL_INCLUDE_DIRS}
    ${LIBKML_INCLUDE_DIRS}
#    ${LibArchive_INCLUDE_DIRS}
)

qt5_add_resources(MAP_RESOURCES map.qrc)

mcc_add_library(mcc-ui-map-lib
    drawables/Drawable.h
    drawables/Flag.cpp
    drawables/Flag.h
    drawables/Interfaces.h
    drawables/Label.cpp
    drawables/Label.h
    drawables/Marker.h
    drawables/MarkerBase.h
    drawables/BiMarker.h
    drawables/Point.h
    drawables/PolyLine.h
    drawables/RulerLabel.h
    drawables/RulerLabel.cpp
    drawables/WithPoints.h
    drawables/WithPosition.h
    drawables/WithRect.h
    drawables/Wrappers.h
    drawables/Zoomable.h
    layers/AircraftLayer.cpp
    layers/AircraftLayer.h
    layers/DeviceLayer.cpp
    layers/DeviceLayer.h
    layers/FlagLayer.cpp
    layers/FlagLayer.h
    layers/GridLayer.cpp
    layers/GridLayer.h
    layers/HomeLayer.cpp
    layers/HomeLayer.h
    layers/InfoLayer.cpp
    layers/InfoLayer.h
    layers/KmlElementLayer.h
    layers/KmlPointLayer.cpp
    layers/KmlPointLayer.h
    layers/KmlPolyLineLayer.h
    layers/KmlPolyLineLayer.cpp
    layers/KmlPolygonLayer.h
    layers/KmlPolygonLayer.cpp
    layers/KmlModelLayer.cpp
    layers/KmlModelLayer.h
    layers/Layer.cpp
    layers/Layer.h
    layers/LayerGroup.cpp
    layers/LayerGroup.h
    layers/MapLayer.cpp
    layers/MapLayer.h
    layers/RulerLayer.cpp
    layers/RulerLayer.h
    layers/SimpleFlagLayer.cpp
    layers/SimpleFlagLayer.h
    layers/TailLayer.cpp
    layers/TailLayer.h
    layers/TemplateLayer.cpp
    layers/TemplateLayer.h
    layers/WaypointLayer.cpp
    layers/WaypointLayer.h
    CacheStackModel.cpp
    CacheStackModel.h
    CacheStackView.cpp
    CacheStackView.h
    CurlDownloader.cpp
    CurlDownloader.h
    DiskCache.cpp
    DiskCache.h
    FileCache.cpp
    FileCache.h
    KmlModel.cpp
    KmlModel.h
    KmlSettings.cpp
    KmlSettings.h
    KmlStyleWidget.cpp
    KmlStyleWidget.h
    KmlUtils.cpp
    KmlUtils.h
    KmlView.cpp
    KmlView.h
    LineAndPos.h
    MapRect.cpp
    MapRect.h
    MapMode.h
    MapSlider.cpp
    MapSlider.h
    MapWidget.cpp
    MapWidget.h
    MapWidgetAnimator.cpp
    MapWidgetAnimator.h
    MemoryCache.cpp
    MemoryCache.h
    MercatorProjection.cpp
    MercatorProjection.h
    OmcfCache.cpp
    OmcfCache.h
    OmcfCacheWidget.cpp
    OmcfCacheWidget.h
    OnlineCache.cpp
    OnlineCache.h
    Ptr.h
    StackCache.cpp
    StackCache.h
    StaticMapType.h
    TilePosCache.h
    TilePosition.h
    WebMapProperties.h
    ${MAP_RESOURCES}
)

set_property(TARGET mcc-ui-map-lib PROPERTY INTERFACE_INCLUDE_DIRECTORIES ${LIBKML_INCLUDE_DIRS})

target_link_libraries(mcc-ui-map-lib
    mcc-misc-lib
    mcc-ui-core-lib
    bmcl
    kmlengine
    kmldom
    ${CURL_LIBRARIES}
#    ${LibArchive_LIBRARIES}
    Qt5::Core
    Qt5::Gui
    Qt5::Widgets
    Qt5::OpenGL
    Qt5::Network
    Qt5::Svg
)

mcc_add_executable(omcf-creator Main.cpp)

target_link_libraries(omcf-creator mcc-ui-map-lib)

set_target_properties(mcc-ui-map-lib
    PROPERTIES
    FOLDER "mcc/ui"
)
