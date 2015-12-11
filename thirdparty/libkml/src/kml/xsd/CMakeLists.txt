set(KMLXSD_SRC
    xsd_complex_type.cc
    xsd_element.cc
    xsd_file.cc
    xsd_handler.cc
    xsd_primitive_type.cc
    xsd_util.cc
    xst_parser.cc
)

add_library(kmlxsd STATIC ${KMLXSD_SRC})

target_link_libraries(kmlxsd kmlbase)
