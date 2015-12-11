cmake_minimum_required(VERSION 2.8.11)

project(qwt)

find_package(Qt5Core REQUIRED)
find_package(Qt5Gui REQUIRED)
find_package(Qt5Widgets REQUIRED)
find_package(Qt5Svg REQUIRED)
find_package(Qt5OpenGL REQUIRED)
find_package(Qt5Concurrent REQUIRED)
find_package(Qt5PrintSupport REQUIRED)

add_subdirectory(src)

get_directory_property(HAS_PARENT_SCOPE PARENT_DIRECTORY)
if(HAS_PARENT_SCOPE)
    set(QWT_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/src PARENT_SCOPE)
    set(QWT_LIBRARIES qwt PARENT_SCOPE)
endif()

if(QWT_BUILD_CPUPLOT)
    qt5_wrap_cpp(MOC_CPP
        examples/cpuplot/cpupiemarker.h
        examples/cpuplot/cpuplot.h
        examples/cpuplot/cpustat.h
    )

    add_executable(cpuplot
        examples/cpuplot/cpupiemarker.cpp
        examples/cpuplot/cpuplot.cpp
        examples/cpuplot/cpustat.cpp
        ${MOC_CPP}
    )

    target_link_libraries(cpuplot
        Qwt
    )
endif()
