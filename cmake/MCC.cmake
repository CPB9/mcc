set(CMAKE_INSTALL_RPATH "${CMAKE_BINARY_DIR}/lib")

function(mcc_install_target target)
    get_target_property(_TARGET_TYPE ${lib} TYPE)
    get_target_property(_TARGET_LOCATION ${lib} LOCATION)
    if(_TARGET_TYPE STREQUAL "SHARED_LIBRARY")
        if(WIN32)
            install(FILES ${_TARGET_LOCATION} DESTINATION bin)
        else()
            install(FILES ${_TARGET_LOCATION} DESTINATION lib)
        endif()
    elseif(_TARGET_TYPE STREQUAL "EXECUTABLE")
        install(FILES ${_TARGET_LOCATION} DESTINATION bin)
    endif()
endfunction()

if(MSVC)
    foreach(flag_var
            CMAKE_CXX_FLAGS CMAKE_CXX_FLAGS_DEBUG CMAKE_CXX_FLAGS_RELEASE
            CMAKE_CXX_FLAGS_MINSIZEREL CMAKE_CXX_FLAGS_RELWITHDEBINFO
            CMAKE_C_FLAGS CMAKE_C_FLAGS_DEBUG CMAKE_C_FLAGS_RELEASE
            CMAKE_C_FLAGS_MINSIZEREL CMAKE_C_FLAGS_RELWITHDEBINFO)
        string(REPLACE "/W3" "" ${flag_var} "${${flag_var}}")
    endforeach()
    if(CMAKE_BUILD_TYPE STREQUAL "Release")
        set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /LTCG")
    endif()
endif()

macro(_add_dep_nocheck name path)
    message(STATUS "Using bundled " ${name})
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/${path} EXCLUDE_FROM_ALL)
    foreach(lib ${ARGN})
        set_target_properties(${lib}
            PROPERTIES
            RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
            LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib
            ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib
            RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/bin
            LIBRARY_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/lib
            ARCHIVE_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/lib
            RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}/bin
            LIBRARY_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}/lib
            ARCHIVE_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}/lib
            FOLDER "libs"
        )
        get_target_property(_TARGET_TYPE ${lib} TYPE)
        if(NOT MSVC)
            if(_TARGET_TYPE STREQUAL "STATIC_LIBRARY")
                set_property(TARGET ${lib} APPEND_STRING PROPERTY COMPILE_FLAGS " -fPIC -DPIC")
            endif()
        endif()
        set_property(TARGET ${lib} APPEND_STRING PROPERTY COMPILE_FLAGS " -w")
        if(WIN32)
            mcc_install_target(${lib})
        endif()
    endforeach()
endmacro()

macro(_add_dep name path found_var)
    if(NOT ${found_var})
    string(TOUPPER ${name} newname)
        if(WIN32)
            _add_dep_nocheck(${name} ${path} ${ARGN})
        elseif(FORCE_BUNDLED_${newname})
            _add_dep_nocheck(${name} ${path} ${ARGN})
        else()
            find_package(${name})
            if(NOT ${found_var})
                _add_dep_nocheck(${name} ${path} ${ARGN})
            endif()
        endif()
    endif()
endmacro()

macro(mcc_add_dep name path)
    if(${name} MATCHES "GTest")
        _add_dep(${name} ${path} GTEST_FOUND gtest)
    elseif(${name} MATCHES "Protobuf")
        _add_dep(${name} ${path} PROTOBUF_FOUND libprotobuf libprotoc protoc)
    elseif(${name} MATCHES "Nanomsg")
        _add_dep(${name} ${path} NANOMSG_FOUND nanomsg)
    elseif(${name} MATCHES "FlatBuffers")
        _add_dep(${name} ${path} FLATBUFFERS_FOUND flatc)
    elseif(${name} MATCHES "CURL")
        _add_dep(${name} ${path} CURL_FOUND libcurl)
    elseif(${name} MATCHES "LibArchive")
        _add_dep_nocheck(${name} ${path} archive)
    elseif(${name} MATCHES "bmcl")
        _add_dep_nocheck(${name} ${path} bmcl)
    elseif(${name} MATCHES "Qwt")
        _add_dep_nocheck(${name} ${path} Qwt)
    elseif(${name} MATCHES "libkml")
        _add_dep_nocheck(${name} ${path} kmlbase kmldom minizip)
    elseif(${name} MATCHES "benchmark")
        _add_dep_nocheck(${name} ${path} benchmark)
    elseif(${name} MATCHES "sfml")
        _add_dep_nocheck(${name} ${path} sfml-system sfml-window)
    elseif(${name} MATCHES "asio")
        _add_dep_nocheck(${name} ${path} asio)
    elseif(${name} MATCHES "photon")
        _add_dep_nocheck(${name} ${path} photon)
    elseif(${name} MATCHES "nmealib")
        _add_dep_nocheck(${name} ${path} nmealib)
    endif()
endmacro()

if (MCC_USE_OPENGL)
    set(_MCC_COMPILE_FLAGS "${CMAKE_CXX_FLAGS} -DMCC_USE_OPENGL")
else()
    set(_MCC_COMPILE_FLAGS ${CMAKE_CXX_FLAGS})
endif()

if(MINGW)
    set(_MCC_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,--as-needed")
elseif(UNIX)
    set(_MCC_LINKER_FLAGS "${_MCC_LINKER_FLAGS} -pthread")
else()
    set(_MCC_LINKER_FLAGS "${_MCC_LINKER_FLAGS}")
endif()

if(UNIX)
    macro(_enable_sanitizer name)
        if(CMAKE_CXX_COMPILER_ID MATCHES "GNU")
            set(_MCC_LINKER_FLAGS "${_MCC_LINKER_FLAGS} -fsanitize=${name} -fno-omit-frame-pointer")
            set(_MCC_COMPILE_FLAGS "${_MCC_COMPILE_FLAGS} -fsanitize=${name} -fno-omit-frame-pointer")
        elseif(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
            set(_MCC_LINKER_FLAGS "${_MCC_LINKER_FLAGS} -fsanitize=${name} -fsanitize=integer -fsanitize=undefined -fno-omit-frame-pointer")
            set(_MCC_COMPILE_FLAGS "${_MCC_COMPILE_FLAGS} -fsanitize=${name} -fsanitize=integer -fsanitize=undefined -fno-omit-frame-pointer")
        endif()
    endmacro()
endif()

set(_MCC_LIBRARIES "")
if (MSVC)
    if (MCC_STACKTRACE)
        find_path(DBGHELP_INCLUDE_DIR NAMES dbghelp.h PATH_SUFFIXES include)
        find_library(DBGHELP_LIBRARY NAMES dbghelp REQUIRED)
        include(FindPackageHandleStandardArgs)
        FIND_PACKAGE_HANDLE_STANDARD_ARGS(DBGHELP DEFAULT_MSG DBGHELP_LIBRARY DBGHELP_INCLUDE_DIR)
        set(DBGHELP_LIBRARIES ${DBGHELP_LIBRARY})
        mark_as_advanced(DBGHELP_LIBRARY DBGHELP_INCLUDE_DIR)
        set(_MCC_COMPILE_FLAGS "${_MCC_COMPILE_FLAGS} -DMCC_STACKTRACE")
        set(_MCC_LIBRARIES "${_MCC_LIBRARIES} ${DBGHELP_LIBRARY}")
        set(_MCC_LINKER_FLAGS "${_MCC_LINKER_FLAGS} /DEBUG /OPT:REF /OPT:ICF")
        set(_MCC_COMPILE_FLAGS "${_MCC_COMPILE_FLAGS} /Zi")
    endif()
endif()

if(SANITIZE_ADDRESS)
    _enable_sanitizer(address)
#elseif(SANITIZE_MEMORY) # instrumented libc++ required
    #_enable_sanitizer(memory)
    #add_definitions(-fsanitize-memory-track-origins=2)
elseif(SANITIZE_THREAD)
    _enable_sanitizer(thread)
endif()

macro(_mcc_setup_properties target)
    if(MSVC)
        set(_MCC_COMPILE_FLAGS "${_MCC_COMPILE_FLAGS} /W3")
    else()
        set(_MCC_COMPILE_FLAGS "${_MCC_COMPILE_FLAGS} -Wall -Wextra -pipe -std=c++11")
        if(CMAKE_BUILD_TYPE STREQUAL "Release")
            set(_MCC_COMPILE_FLAGS "${_MCC_COMPILE_FLAGS}") #-march=i686)
        else()
            set(_MCC_COMPILE_FLAGS "${_MCC_COMPILE_FLAGS} -O0 -ggdb3")
        endif()
    endif()
    set_target_properties(${target}
        PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
        LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib
        ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib
        RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/bin
        LIBRARY_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/lib
        ARCHIVE_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/lib
        RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}/bin
        LIBRARY_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}/lib
        ARCHIVE_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}/lib
        COMPILE_FLAGS ${_MCC_COMPILE_FLAGS}
    )
    if(_MCC_LINKER_FLAGS)
        set_target_properties(${target}
            PROPERTIES
            LINK_FLAGS ${_MCC_LINKER_FLAGS}
        )
    endif()
    get_target_property(_TARGET_EXCLUDE_FLOM_ALL ${target} EXCLUDE_FROM_ALL)
    if(NOT _TARGET_EXCLUDE_FLOM_ALL)
        install(TARGETS ${target}
                RUNTIME DESTINATION bin
                LIBRARY DESTINATION lib
                ARCHIVE DESTINATION lib
        )
    endif()
    if (_MCC_LIBRARIES)
        target_link_libraries(${target} ${DBGHELP_LIBRARY})
    endif()
endmacro()

macro(mcc_add_static_library target)
    add_library(${target} STATIC ${ARGN})

    _mcc_setup_properties(${target})
endmacro()

macro(mcc_add_shared_library target)
    add_library(${target} SHARED ${ARGN})

    _mcc_setup_properties(${target})
endmacro()

macro(mcc_add_library target)
    if(WIN32)
        mcc_add_static_library(${target} ${ARGN})
    elseif(APPLE)
        mcc_add_static_library(${target} ${ARGN})
    else()
        mcc_add_static_library(${target} ${ARGN})
    endif()
endmacro()

macro(mcc_add_executable target)
    add_executable(${target} ${ARGN})

    _mcc_setup_properties(${target})
endmacro()

