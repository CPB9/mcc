set(KMLENGINE_SRC
    clone.cc
    entity_mapper.cc
    feature_balloon.cc
    feature_view.cc
    feature_visitor.cc
    find.cc
    find_xml_namespaces.cc
    get_link_parents.cc
    get_links.cc
    href.cc
    id_mapper.cc
    kml_cache.cc
    kml_file.cc
    kml_stream.cc
    kml_uri.cc
    kmz_cache.cc
    kmz_file.cc
    link_util.cc
    location_util.cc
    merge.cc
    parse_old_schema.cc
    style_inliner.cc
    style_merger.cc
    style_resolver.cc
    style_splitter.cc
    update_processor.cc
    update.cc
)

add_library(kmlengine STATIC ${KMLENGINE_SRC})

target_link_libraries(kmlengine kmlbase kmldom)
