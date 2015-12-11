include_directories(
    SYSTEM
    ${BENCHMARK_INCLUDE_DIRS}
)

set(BENCH_DIR ${CMAKE_BINARY_DIR}/bin/bench)
file(MAKE_DIRECTORY ${BENCH_DIR})

macro(add_bench test file)
    mcc_add_executable(${test} ${file})
    target_link_libraries(${test}
        ${ARGN}
        ${BENCHMARK_LIBRARIES}
    )

    set_target_properties(${test}
        PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${BENCH_DIR}
        FOLDER "bench"
    )
endmacro()

add_bench(option-bench Option.cpp mcc-misc-lib)
