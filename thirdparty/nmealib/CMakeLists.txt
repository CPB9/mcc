cmake_minimum_required (VERSION 2.8)
project (nmealib)

include_directories(
    SYSTEM
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

set(INCLUDES 
    include/nmea/config.h
    include/nmea/context.h
    include/nmea/generate.h
    include/nmea/generator.h
    include/nmea/gmath.h
    include/nmea/info.h
    include/nmea/nmea.h
    include/nmea/parse.h
    include/nmea/parser.h
    include/nmea/sentence.h
    include/nmea/time.h
    include/nmea/tok.h
    include/nmea/units.h
    include/nmea/strtod.h
)

set(SOURCES
    src/context.c
    src/generate.c
    src/generator.c
    src/gmath.c
    src/info.c
    src/parse.c
    src/parser.c
    src/sentence.c
    src/time.c
    src/tok.c
    src/strtod.cc
)

add_library(nmealib
    ${INCLUDES}
    ${SOURCES}
)

get_directory_property(HAS_PARENT_SCOPE PARENT_DIRECTORY)
if(HAS_PARENT_SCOPE)
    set(NMEA_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/include PARENT_SCOPE)
endif()