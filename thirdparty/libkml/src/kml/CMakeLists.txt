include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${EXPAT_INCLUDE_DIRS}
)

#if(NOT MSVC)
#    add_definitions(-fno-rtti)
#endif()

add_subdirectory(base)
add_subdirectory(convenience)
add_subdirectory(dom)
add_subdirectory(engine)
add_subdirectory(regionator)
add_subdirectory(xsd)
