function(add_kmlregionator_test name)
    add_libkml_test(${name} kmlregionator)
endfunction()

add_kmlregionator_test(feature_list_region_handler_test)
add_kmlregionator_test(regionator_qid_test)
add_kmlregionator_test(regionator_test)
add_kmlregionator_test(regionator_util_test)
