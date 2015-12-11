project(photon)

cmake_minimum_required(VERSION 2.8.7)
enable_testing()
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

get_directory_property(HAS_PARENT_SCOPE PARENT_DIRECTORY)
if(HAS_PARENT_SCOPE)
    set(PHOTON_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/src PARENT_SCOPE)
endif()

if(MSVC)
    foreach (flag_var
             CMAKE_CXX_FLAGS CMAKE_CXX_FLAGS_DEBUG CMAKE_CXX_FLAGS_RELEASE
             CMAKE_CXX_FLAGS_MINSIZEREL CMAKE_CXX_FLAGS_RELWITHDEBINFO)
        string(REPLACE "/W3" "/W4" ${flag_var} "${${flag_var}}")
    endforeach()
else()
    add_definitions(-Wall -Wextra -pipe)
    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        add_definitions(-O0 -ggdb3)
    endif()
endif()

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src)
add_subdirectory(src/photon)

if(NOT HAS_PARENT_SCOPE)
    set(GTEST_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/gtest/include)
    add_subdirectory(tests)
endif()

if(HAS_PARENT_SCOPE)
    set(PHOTON_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR} PARENT_SCOPE)
    set(PHOTON_LIBRARIES photon PARENT_SCOPE)
endif()
