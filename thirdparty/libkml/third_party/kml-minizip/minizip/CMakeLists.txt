include_directories(${ZLIB_INCLUDE_DIRS})

set(MINIZIP_SRC
    unzip.c
    ioapi.c
    iomem_simple.c
    zip.c
)

add_library(minizip ${MINIZIP_SRC})

target_link_libraries(minizip ${ZLIB_LIBRARIES})

