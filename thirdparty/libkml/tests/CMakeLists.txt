include_directories(${GTEST_INCLUDE_DIRS})

set(TEST_DATA_DIR ${CMAKE_CURRENT_SOURCE_DIR}/testdata)
add_definitions(-DDATADIR="${TEST_DATA_DIR}")

function(add_libkml_test name)
    add_executable(${name} ${name}.cc)
    target_link_libraries(${name}
        ${ARGN}
        ${GTEST_LIBRARIES}
        ${GTEST_MAIN_LIBRARIES}
    )
    add_test(${name} ${name})
endfunction()

add_subdirectory(base)
add_subdirectory(convenience)
add_subdirectory(dom)
add_subdirectory(regionator)
add_subdirectory(engine)
add_subdirectory(xsd)

