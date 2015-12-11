include_directories(
    SYSTEM
    ${BMCL_INCLUDE_DIRS}
    ${GTEST_INCLUDE_DIRS}
    ${PEGTL_INCLUDE_DIRS}
)

set(TESTS_DIR ${CMAKE_BINARY_DIR}/bin/tests)
file(MAKE_DIRECTORY ${TESTS_DIR})

set(ALL_TESTS_SRC "")
set(ALL_TESTS_TARGETS "")

macro(add_unit_test test file)
    mcc_add_executable(${test} ${file})
    target_link_libraries(${test}
        ${ARGN}
        Qt5::Core
        ${GTEST_LIBRARIES}
        ${GTEST_MAIN_LIBRARIES}
    )

    set_target_properties(${test}
        PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${TESTS_DIR}
        RUNTIME_OUTPUT_DIRECTORY_RELEASE ${TESTS_DIR}
        RUNTIME_OUTPUT_DIRECTORY_DEBUG ${TESTS_DIR}
        FOLDER "tests"
    )

    add_test(${test} ${TESTS_DIR}/${test})
    list(APPEND ALL_TESTS_SRC ${file})
    list(APPEND ALL_TESTS_TARGETS ${test})
endmacro()

add_unit_test(misc-tests Misc.cpp mcc-misc-lib)
add_unit_test(channel-tests Channel.cpp mcc-misc-lib)
add_unit_test(map-tests Map.cpp mcc-ui-map-lib)
add_unit_test(sharedvar-tests SharedVar.cpp mcc-misc-lib)
add_definitions(-DTEST_DATABASE="${CMAKE_CURRENT_SOURCE_DIR}/../src/mcc/target/db/local.sqlite")
add_definitions(-DTEST_DECODE_DIR="${CMAKE_CURRENT_SOURCE_DIR}/../src/mcc/target/decode/")
add_unit_test(decode-tests Decode.cpp mcc-core-decode-lib)
#add_unit_test(s57-tests S57.cpp mcc-ui-s57-lib Qt5::Core)

mcc_add_executable(all_tests EXCLUDE_FROM_ALL ${ALL_TESTS_SRC})
target_link_libraries(all_tests
    mcc-misc-lib
    mcc-ui-map-lib
    mcc-core-decode-lib
    Qt5::Core
    Qt5::Sql
    ${GTEST_LIBRARIES}
    ${GTEST_MAIN_LIBRARIES}
)

set_target_properties(all_tests
    PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${TESTS_DIR}
    RUNTIME_OUTPUT_DIRECTORY_RELEASE ${TESTS_DIR}
    RUNTIME_OUTPUT_DIRECTORY_DEBUG ${TESTS_DIR}
)

add_custom_target(tests DEPENDS ${ALL_TESTS_TARGETS})
