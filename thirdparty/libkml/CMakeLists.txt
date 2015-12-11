cmake_minimum_required(VERSION 2.8.11)

project(libkml)

option(LIBKML_BUILD_TESTS "Build libkml tests" OFF)

if(NOT WIN32)
    add_definitions(-fPIC -DPIC)
endif()

if(MSVC)
    add_definitions(/D_CRT_SECURE_NO_WARNINGS=1)
endif()

find_package(ZLIB)
if(NOT ${ZLIB_FOUND})
    add_subdirectory(third_party/zlib-1.2.8 EXCLUDE_FROM_ALL)
    set(ZLIB_INCLUDE_DIRS
        ${CMAKE_CURRENT_SOURCE_DIR}/third_party/zlib-1.2.8
        ${CMAKE_CURRENT_BINARY_DIR}/third_party/zlib-1.2.8
    )
    set(ZLIB_LIBRARIES zlibstatic)
endif()

add_subdirectory(third_party/kml-minizip/minizip)
add_subdirectory(third_party/uriparser-0.7.5)

find_package(EXPAT)
if(NOT ${EXPAT_FOUND})
    add_subdirectory(third_party/expat-2.1.0 EXCLUDE_FROM_ALL)
    set(EXPAT_INCLUDE_DIRS
        ${CMAKE_CURRENT_SOURCE_DIR}/third_party/expat-2.1.0/lib
        ${CMAKE_CURRENT_BINARY_DIR}/third_party/expat-2.1.0
    )
    set(EXPAT_LIBRARIES expat)
    add_definitions(-DXML_STATIC)
endif()

set(LIBKML_DEP_INCLUDES
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party/kml-minizip
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party/uriparser-0.7.5/include
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party/boost_1_57_0
    ${ZLIB_INCLUDE_DIRS}
    ${EXPAT_INCLUDE_DIRS}
)

include_directories(${LIBKML_DEP_INCLUDES})

add_subdirectory(src/kml)

if(LIBKML_BUILD_TESTS)
    enable_testing()
    find_package(GTest)
    if(NOT ${GTEST_FOUND})
        add_subdirectory(third_party/gtest-1.7.0)
    endif()
    add_subdirectory(tests)
endif()

get_directory_property(HAS_PARENT_SCOPE PARENT_DIRECTORY)
if(HAS_PARENT_SCOPE)
    set(LIBKML_INCLUDE_DIRS
        ${CMAKE_CURRENT_SOURCE_DIR}/src
        ${LIBKML_DEP_INCLUDES}
        PARENT_SCOPE
    )
endif()

