set(KMLCONVENIENCE_SRC
    atom_util.cc
    convenience.cc
    csv_file.cc
    csv_parser.cc
    feature_list.cc
    google_doc_list.cc
    google_maps_data.cc
    google_picasa_web.cc
    google_spreadsheets.cc
    http_client.cc
    kmz_check_links.cc
)

add_library(kmlconvenience STATIC ${KMLCONVENIENCE_SRC})

target_link_libraries(kmlconvenience kmlbase kmldom kmlengine)
