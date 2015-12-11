if(WIN32)
    set(KMLBASE_FILE_SRC
        file_win32.cc
        missing/strptime.cc
    )
else()
    set(KMLBASE_FILE_SRC file_posix.cc)
endif()

set(KMLBASE_SRC
    attributes.cc
    csv_splitter.cc
    date_time.cc
    expat_handler_ns.cc
    expat_parser.cc
    file.cc
    math_util.cc
    mimetypes.cc
    referent.cc
    string_util.cc
    time_util.cc
    uri_parser.cc
    version.cc
    xml_namespaces.cc
    zip_file.cc
    missing/strtod.cc
)

add_library(kmlbase STATIC
    ${KMLBASE_SRC}
    ${KMLBASE_FILE_SRC}
)

target_link_libraries(kmlbase
    ${EXPAT_LIBRARIES}
    minizip
    uriparser
)
