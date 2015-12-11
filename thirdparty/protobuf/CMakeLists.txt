cmake_minimum_required(VERSION 2.8)

project(protobuf)

if(MSVC)
    file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/vsprojects/config.h DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
    add_definitions(/DHAVE_CONFIG_H /D_CRT_SECURE_NO_WARNINGS)
else()
    include(CheckCXXSourceCompiles)
    check_cxx_source_compiles("
        #if defined(__GNUC__) && (__GNUC__ < 4 || (__GNUC__ == 4 && __GNUC_MINOR__ < 2))
            # error GCC too old for unordered_map
        #endif"
        STL_HASH_OLD_GCC
    )
    message(STATUS "Checking the location of hash_map")
    foreach(location unordered_map tr1/unordered_map)
        foreach(namespace std std::tr1)
            if(NOT HAVE_HASH_MAP OR STL_HASH_OLD_GCC)
                set(HASH_MAP_H ${location} CACHE INTERNAL "Found hashmap header <${location}>")
                set(HASH_NAMESPACE ${namespace} CACHE INTERNAL "Found hashmap namespace <${namespace}>")
                set(HASH_MAP_CLASS unordered_map CACHE INTERNAL "Found hashmap class unordered_map")
                set(CMAKE_REQUIRED_FLAGS -std=c++11)
                check_cxx_source_compiles("
                    #include <${location}>
                    int main()
                    {
                        const ${namespace}::unordered_map<int, int> t;
                        return t.find(5) == t.end();
                    }"
                    HAVE_HASH_MAP
                )
            endif()
        endforeach()
    endforeach()
    unset(CMAKE_REQUIRED_FLAGS)
    if(HAVE_HASH_MAP)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
    endif()

    foreach(location ext/hash_map hash_map)
        foreach(namespace __gnu_cxx "" std stdext)
            if(NOT HAVE_HASH_MAP)
                set(HASH_MAP_H ${location} CACHE INTERNAL "Found hashmap header <${HASH_MAP_H}>")
                set(HASH_NAMESPACE ${namespace} CACHE INTERNAL "Found hashmap namespace - ${namespace}")
                set(HASH_MAP_CLASS hash_map CACHE INTERNAL "Found hashmap class hash_map")
                check_cxx_source_compiles("
                    #include <${location}>
                    int main()
                    {
                        ${namespace}::hash_map<int, int> t;
                    }"
                    HAVE_HASH_MAP
                )
            endif()
        endforeach()
    endforeach()
    if(HAVE_HASH_MAP)
        set(HAVE_HASH_SET ${HAVE_HASH_MAP})
        string(REPLACE "map" "set" HASH_SET_H "${HASH_MAP_H}")
        string(REPLACE "map" "set" HASH_SET_CLASS "${HASH_MAP_CLASS}")
        message(STATUS "Found hash_map ${HASH_NAMESPACE}::${HASH_MAP_H} in header <${HASH_MAP_H}>")
        message(STATUS "Found hash_set ${HASH_NAMESPACE}::${HASH_SET_H} in header <${HASH_SET_H}>")
    else()
        unset(HASH_NAMESPACE)
        unset(HASH_MAP_CLASS)
    endif()

    include(CheckIncludeFiles)
    include(CheckFunctionExists)

    check_include_files(pthread.h HAVE_PTHREAD)
    check_include_files(fcntl.h HAVE_FCNTL_H)
    check_include_files(inttypes.h HAVE_INTTYPES_H)
    check_include_files(limits.h HAVE_LIMITS_H)
    check_include_files(stdlib.h HAVE_STDLIB_H)
    check_include_files(unistd.h HAVE_UNISTD_H)

    check_function_exists(ftruncate  HAVE_FTRUNCATE)
    check_function_exists(memset HAVE_MEMSET)
    check_function_exists(mkdir HAVE_MKDIR)
    check_function_exists(strchr HAVE_STRCHR)
    check_function_exists(strerror HAVE_STRERROR)
    check_function_exists(strtol HAVE_STRTOL)

    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/cmake/config.h.in ${CMAKE_CURRENT_BINARY_DIR}/config.h)
    add_definitions(-DHAVE_CONFIG_H -Wall -Wwrite-strings -Woverloaded-virtual -Wno-sign-compare -D_GNU_SOURCE=1)
endif()

include_directories(${CMAKE_CURRENT_BINARY_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src)

set(PROTOBUF_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src/google/protobuf)

set(LIBPROTOBUF_LITE_SRC
    ${PROTOBUF_DIR}/stubs/atomicops_internals_x86_gcc.cc
    ${PROTOBUF_DIR}/stubs/atomicops_internals_x86_msvc.cc
    ${PROTOBUF_DIR}/stubs/common.cc
    ${PROTOBUF_DIR}/stubs/once.cc
    ${PROTOBUF_DIR}/stubs/hash.h
    ${PROTOBUF_DIR}/stubs/map_util.h
    ${PROTOBUF_DIR}/stubs/shared_ptr.h
    ${PROTOBUF_DIR}/stubs/stringprintf.cc
    ${PROTOBUF_DIR}/stubs/stringprintf.h
    ${PROTOBUF_DIR}/extension_set.cc
    ${PROTOBUF_DIR}/generated_message_util.cc
    ${PROTOBUF_DIR}/message_lite.cc
    ${PROTOBUF_DIR}/repeated_field.cc
    ${PROTOBUF_DIR}/wire_format_lite.cc
    ${PROTOBUF_DIR}/io/coded_stream.cc
    ${PROTOBUF_DIR}/io/coded_stream_inl.h
    ${PROTOBUF_DIR}/io/zero_copy_stream.cc
    ${PROTOBUF_DIR}/io/zero_copy_stream_impl_lite.cc
)

set(LIBPROTOBUF_SRC
    ${LIBPROTOBUF_LITE_SRC}
    ${PROTOBUF_DIR}/stubs/strutil.cc
    ${PROTOBUF_DIR}/stubs/strutil.h
    ${PROTOBUF_DIR}/stubs/substitute.cc
    ${PROTOBUF_DIR}/stubs/substitute.h
    ${PROTOBUF_DIR}/stubs/structurally_valid.cc
    ${PROTOBUF_DIR}/descriptor.cc
    ${PROTOBUF_DIR}/descriptor.pb.cc
    ${PROTOBUF_DIR}/descriptor_database.cc
    ${PROTOBUF_DIR}/dynamic_message.cc
    ${PROTOBUF_DIR}/extension_set_heavy.cc
    ${PROTOBUF_DIR}/generated_message_reflection.cc
    ${PROTOBUF_DIR}/message.cc
    ${PROTOBUF_DIR}/reflection_ops.cc
    ${PROTOBUF_DIR}/service.cc
    ${PROTOBUF_DIR}/text_format.cc
    ${PROTOBUF_DIR}/unknown_field_set.cc
    ${PROTOBUF_DIR}/wire_format.cc
    ${PROTOBUF_DIR}/io/gzip_stream.cc
    ${PROTOBUF_DIR}/io/printer.cc
    ${PROTOBUF_DIR}/io/strtod.cc
    ${PROTOBUF_DIR}/io/tokenizer.cc
    ${PROTOBUF_DIR}/io/zero_copy_stream_impl.cc
    ${PROTOBUF_DIR}/compiler/importer.cc
    ${PROTOBUF_DIR}/compiler/parser.cc
)

add_library(libprotobuf ${LIBPROTOBUF_SRC})
if(NOT MSVC)
    set_target_properties(libprotobuf
        PROPERTIES
        OUTPUT_NAME "protobuf"
    )
endif()

set(LIBPROTOC_SRC
    ${PROTOBUF_DIR}/compiler/code_generator.cc
    ${PROTOBUF_DIR}/compiler/command_line_interface.cc
    ${PROTOBUF_DIR}/compiler/plugin.cc
    ${PROTOBUF_DIR}/compiler/plugin.pb.cc
    ${PROTOBUF_DIR}/compiler/subprocess.cc
    ${PROTOBUF_DIR}/compiler/subprocess.h
    ${PROTOBUF_DIR}/compiler/zip_writer.cc
    ${PROTOBUF_DIR}/compiler/zip_writer.h
    ${PROTOBUF_DIR}/compiler/cpp/cpp_enum.cc
    ${PROTOBUF_DIR}/compiler/cpp/cpp_enum.h
    ${PROTOBUF_DIR}/compiler/cpp/cpp_enum_field.cc
    ${PROTOBUF_DIR}/compiler/cpp/cpp_enum_field.h
    ${PROTOBUF_DIR}/compiler/cpp/cpp_extension.cc
    ${PROTOBUF_DIR}/compiler/cpp/cpp_extension.h
    ${PROTOBUF_DIR}/compiler/cpp/cpp_field.cc
    ${PROTOBUF_DIR}/compiler/cpp/cpp_field.h
    ${PROTOBUF_DIR}/compiler/cpp/cpp_file.cc
    ${PROTOBUF_DIR}/compiler/cpp/cpp_file.h
    ${PROTOBUF_DIR}/compiler/cpp/cpp_generator.cc
    ${PROTOBUF_DIR}/compiler/cpp/cpp_helpers.cc
    ${PROTOBUF_DIR}/compiler/cpp/cpp_helpers.h
    ${PROTOBUF_DIR}/compiler/cpp/cpp_message.cc
    ${PROTOBUF_DIR}/compiler/cpp/cpp_message.h
    ${PROTOBUF_DIR}/compiler/cpp/cpp_message_field.cc
    ${PROTOBUF_DIR}/compiler/cpp/cpp_message_field.h
    ${PROTOBUF_DIR}/compiler/cpp/cpp_options.h
    ${PROTOBUF_DIR}/compiler/cpp/cpp_primitive_field.cc
    ${PROTOBUF_DIR}/compiler/cpp/cpp_primitive_field.h
    ${PROTOBUF_DIR}/compiler/cpp/cpp_service.cc
    ${PROTOBUF_DIR}/compiler/cpp/cpp_service.h
    ${PROTOBUF_DIR}/compiler/cpp/cpp_string_field.cc
    ${PROTOBUF_DIR}/compiler/cpp/cpp_string_field.h
    ${PROTOBUF_DIR}/compiler/java/java_context.cc
    ${PROTOBUF_DIR}/compiler/java/java_context.h
    ${PROTOBUF_DIR}/compiler/java/java_enum.cc
    ${PROTOBUF_DIR}/compiler/java/java_enum.h
    ${PROTOBUF_DIR}/compiler/java/java_enum_field.cc
    ${PROTOBUF_DIR}/compiler/java/java_enum_field.h
    ${PROTOBUF_DIR}/compiler/java/java_extension.cc
    ${PROTOBUF_DIR}/compiler/java/java_extension.h
    ${PROTOBUF_DIR}/compiler/java/java_field.cc
    ${PROTOBUF_DIR}/compiler/java/java_field.h
    ${PROTOBUF_DIR}/compiler/java/java_file.cc
    ${PROTOBUF_DIR}/compiler/java/java_file.h
    ${PROTOBUF_DIR}/compiler/java/java_generator.cc
    ${PROTOBUF_DIR}/compiler/java/java_generator_factory.cc
    ${PROTOBUF_DIR}/compiler/java/java_generator_factory.h
    ${PROTOBUF_DIR}/compiler/java/java_helpers.cc
    ${PROTOBUF_DIR}/compiler/java/java_helpers.h
    ${PROTOBUF_DIR}/compiler/java/java_lazy_message_field.cc
    ${PROTOBUF_DIR}/compiler/java/java_lazy_message_field.h
    ${PROTOBUF_DIR}/compiler/java/java_message.cc
    ${PROTOBUF_DIR}/compiler/java/java_message.h
    ${PROTOBUF_DIR}/compiler/java/java_message_field.cc
    ${PROTOBUF_DIR}/compiler/java/java_message_field.h
    ${PROTOBUF_DIR}/compiler/java/java_name_resolver.cc
    ${PROTOBUF_DIR}/compiler/java/java_name_resolver.h
    ${PROTOBUF_DIR}/compiler/java/java_primitive_field.cc
    ${PROTOBUF_DIR}/compiler/java/java_primitive_field.h
    ${PROTOBUF_DIR}/compiler/java/java_shared_code_generator.cc
    ${PROTOBUF_DIR}/compiler/java/java_shared_code_generator.h
    ${PROTOBUF_DIR}/compiler/java/java_service.cc
    ${PROTOBUF_DIR}/compiler/java/java_service.h
    ${PROTOBUF_DIR}/compiler/java/java_string_field.cc
    ${PROTOBUF_DIR}/compiler/java/java_string_field.h
    ${PROTOBUF_DIR}/compiler/java/java_doc_comment.cc
    ${PROTOBUF_DIR}/compiler/java/java_doc_comment.h
    ${PROTOBUF_DIR}/compiler/python/python_generator.cc
)

add_library(libprotoc ${LIBPROTOC_SRC})
target_link_libraries(libprotoc libprotobuf)
if(NOT MSVC)
    set_target_properties(libprotoc
        PROPERTIES
        OUTPUT_NAME "protoc"
    )
endif()

set(PROTOC_SRC
    ${PROTOBUF_DIR}/compiler/main.cc
)

add_executable(protoc ${PROTOC_SRC})
target_link_libraries(protoc libprotoc)

get_directory_property(HAS_PARENT_SCOPE PARENT_DIRECTORY)
if(HAS_PARENT_SCOPE)
    set(PROTOBUF_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/src ${CMAKE_CURRENT_BINARY_DIR} PARENT_SCOPE)
    set(PROTOBUF_LIBRARIES libprotobuf PARENT_SCOPE)
    set(PROTOBUF_PROTOC_LIBRARIES libprotoc PARENT_SCOPE)
    set(PROTOBUF_PROTOC_EXECUTABLE protoc PARENT_SCOPE)
endif()

