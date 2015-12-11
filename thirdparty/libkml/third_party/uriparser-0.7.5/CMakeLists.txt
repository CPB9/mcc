set(URIPARSER_SRC
    lib/UriCommon.c
    lib/UriCommon.h
    lib/UriCompare.c
    lib/UriEscape.c
    lib/UriFile.c
    lib/UriIp4.c
    lib/UriIp4Base.c
    lib/UriIp4Base.h
    lib/UriNormalize.c
    lib/UriNormalizeBase.c
    lib/UriNormalizeBase.h
    lib/UriParse.c
    lib/UriParseBase.c
    lib/UriParseBase.h
    lib/UriQuery.c
    lib/UriRecompose.c
    lib/UriResolve.c
    lib/UriShorten.c
)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

add_library(uriparser ${URIPARSER_SRC})

