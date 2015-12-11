project(asio)

cmake_minimum_required(VERSION 2.8.11)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/asio/include)

if(MSVC)
    set(ASIO_FLAGS -DASIO_SEPARATE_COMPILATION -DASIO_STANDALONE -D_WIN32_WINNT=0x0501 -DASIO_ENABLE_CANCELIO)
else()
    set(ASIO_FLAGS -DASIO_SEPARATE_COMPILATION -DASIO_STANDALONE -pthread -std=c++11)

endif()

add_definitions(${ASIO_FLAGS})

add_library(asio
    ${CMAKE_CURRENT_SOURCE_DIR}/asio/src/asio.cpp
)

set_property(TARGET asio PROPERTY INTERFACE_COMPILE_OPTIONS ${ASIO_FLAGS})

get_directory_property(HAS_PARENT_SCOPE PARENT_DIRECTORY)
if(HAS_PARENT_SCOPE)
    set(ASIO_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/asio/include PARENT_SCOPE)
    set(ASIO_LIBRARIES asio PARENT_SCOPE)
endif()

