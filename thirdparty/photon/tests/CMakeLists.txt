if(NOT MSVC)
    add_definitions(-std=c++11)
endif()

include_directories(
    ${PHOTON_INCLUDE_DIRS}
    SYSTEM
    ${GTEST_INCLUDE_DIRS}
)

set(TESTS_DIR ${CMAKE_BINARY_DIR}/bin/tests)
file(MAKE_DIRECTORY ${TESTS_DIR})

macro(add_unit_test test file)
    add_executable(${test} ${file})
    target_link_libraries(${test}
        ${ARGN}
        photon
        gtest
        gtest_main
    )

    set_target_properties(${test}
        PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${TESTS_DIR}
        FOLDER "tests"
    )
    add_test(${test} ${TESTS_DIR}/${test})
endmacro()

add_unit_test(reader_tests Reader.cpp)
