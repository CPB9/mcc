cmake_minimum_required(VERSION 2.8.11)

cmake_policy(SET CMP0020 OLD)
if(POLICY CMP0026)
    cmake_policy(SET CMP0026 OLD)
endif()

project(mcc)

enable_testing()

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake)
include(MCC)
include(DeployQt5)

set(MCC_BACKEND_NAME)
if(MCC_USE_OPENGL)
    set(MCC_BACKEND_NAME "-opengl")
endif()

set(CPACK_PACKAGE_VERSION 0.1)
set(CPACK_PACKAGE_EXECUTABLES "mcc-ui-ude" "МПУ")
#set(CPACK_NSIS_MODIFY_PATH "ON")
set(CPACK_PACKAGE_VENDOR "Applmech")
set(CPACK_NSIS_ENABLE_UNINSTALL_BEFORE_INSTALL 1)
set(CPACK_PACKAGE_NAME "mcc${MCC_BACKEND_NAME}")
set(CPACK_RESOURCE_FILE_LICENSE_PROVIDED 0)
set(CPACK_PACKAGE_INSTALL_DIRECTORY "mcc")
set(CPACK_PACKAGE_INSTALL_REGISTRY_KEY "applmech-mcc")

include(CPack)

mcc_add_dep(GTest thirdparty/gtest)
mcc_add_dep(Protobuf thirdparty/protobuf)
#mcc_add_dep(Nanomsg thirdparty/nanomsg)
#mcc_add_dep(FlatBuffers thirdparty/flatbuffers)
mcc_add_dep(CURL thirdparty/curl)
#mcc_add_dep(LibArchive thirdparty/libarchive)
mcc_add_dep(bmcl thirdparty/bmcl)
mcc_add_dep(Qwt thirdparty/qwt)
mcc_add_dep(libkml thirdparty/libkml)
mcc_add_dep(sfml thirdparty/sfml)
mcc_add_dep(asio thirdparty/asio)
mcc_add_dep(photon thirdparty/photon)
mcc_add_dep(nmealib thirdparty/nmealib)
add_subdirectory(thirdparty/mavlink)

if (NOT MSVC)
    mcc_add_dep(benchmark thirdparty/benchmark)
endif()
set(PEGTL_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/pegtl)

find_package(Qt5Core REQUIRED)
find_package(Qt5Gui REQUIRED)
find_package(Qt5Widgets REQUIRED)
find_package(Qt5Sql REQUIRED)
find_package(Qt5Svg REQUIRED)
find_package(Qt5OpenGL REQUIRED)
find_package(Qt5Network REQUIRED)
find_package(Qt5Multimedia REQUIRED)
find_package(Qt5Quick REQUIRED)
find_package(Qt5SerialPort REQUIRED)
find_package(Qt5PrintSupport REQUIRED)
#TODO: добавить деплой Qt ANGLE (libEGL)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src)
include_directories(SYSTEM ${CMAKE_CURRENT_BINARY_DIR}/src)
add_subdirectory(src/mcc)

if(WIN32)
    if(MINGW)
        install_qt_mingw_rt(bin)
    endif()
    install_qt5_platform(bin)
    install_qt5_icu(bin)
    install_qt5_lib(bin
        Core
        Gui
        Widgets
        Sql
        OpenGL
        Network
        Multimedia
        Svg
        Quick
        Qml
        PrintSupport
        SerialPort
    )
    install_qt5_dbdrivers(bin
        qsqlite
    )
    install_qt5_imageformats(bin
        qjpeg
        qsvg
        qico
    )
    install_qt5_qml_plugin_all(bin)
    install_qt5_translations("ru" "bin/translations")

    if (CMAKE_CL_64)
        file(COPY "${CMAKE_CURRENT_SOURCE_DIR}/src/mcc/ui/vimba/AVTImageTransform/Bin/Win64/AVTImageTransform.dll" DESTINATION "${CMAKE_CURRENT_BINARY_DIR}/bin")
        install(FILES "${CMAKE_CURRENT_BINARY_DIR}/bin/AVTImageTransform.dll" DESTINATION bin)
        file(COPY "${CMAKE_CURRENT_SOURCE_DIR}/src/mcc/ui/vimba/VimbaC/Bin/Win64/VimbaC.dll" DESTINATION "${CMAKE_CURRENT_BINARY_DIR}/bin")
        install(FILES "${CMAKE_CURRENT_BINARY_DIR}/bin/VimbaC.dll" DESTINATION bin)
    else()
        file(COPY "${CMAKE_CURRENT_SOURCE_DIR}/src/mcc/ui/vimba/AVTImageTransform/Bin/Win32/AVTImageTransform.dll" DESTINATION "${CMAKE_CURRENT_BINARY_DIR}/bin")
        install(FILES "${CMAKE_CURRENT_BINARY_DIR}/bin/AVTImageTransform.dll" DESTINATION bin)
        file(COPY "${CMAKE_CURRENT_SOURCE_DIR}/src/mcc/ui/vimba/VimbaC/Bin/Win32/VimbaC.dll" DESTINATION "${CMAKE_CURRENT_BINARY_DIR}/bin")
        install(FILES "${CMAKE_CURRENT_BINARY_DIR}/bin/VimbaC.dll" DESTINATION bin)
    endif()
endif()

function(install_mcc_files relpath dest expr)
    file(GLOB_RECURSE _QML_FILES "${CMAKE_CURRENT_SOURCE_DIR}/${relpath}/${expr}")
    foreach(_FILE ${_QML_FILES})
        string(LENGTH ${CMAKE_CURRENT_SOURCE_DIR}/${relpath} _FILE_LENGTH)
        string(SUBSTRING ${_FILE} ${_FILE_LENGTH} -1 _RELATIVE_FILE)
        get_filename_component(_RELATIVE_DIR ${_RELATIVE_FILE} DIRECTORY)
        install(FILES ${_FILE} DESTINATION ${dest}${_RELATIVE_DIR})
    endforeach()
endfunction()

install_mcc_files("src/mcc/ui/examples" "bin/examples" "*")
install_mcc_files("src/mcc/target/db" "bin/db" "*sqlite")

file(COPY "${CMAKE_CURRENT_SOURCE_DIR}/src/mcc/target/db/local.sqlite" DESTINATION "${CMAKE_CURRENT_BINARY_DIR}/bin")
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/bin/local.sqlite" DESTINATION bin)

file(GLOB DECODE_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/src/mcc/target/decode/*.decode")
file(COPY ${DECODE_SOURCES} DESTINATION "${CMAKE_CURRENT_BINARY_DIR}/bin/decode")
file(GLOB DECODE_SOURCES "${CMAKE_CURRENT_BINARY_DIR}/bin/decode/*.decode")
install(FILES ${DECODE_SOURCES} DESTINATION bin/decode)

add_subdirectory(tests)
if (NOT MSVC)
    add_subdirectory(bench EXCLUDE_FLOM_ALL)
endif()

