set(PROTO)
set(PROTO_GEN_FILES)
set(SOURCES)

macro(generate_proto_messages)
    foreach(name ${ARGN})
        add_custom_command(
            OUTPUT
            ${CMAKE_CURRENT_BINARY_DIR}/protobuf/${name}.pb.h
            ${CMAKE_CURRENT_BINARY_DIR}/protobuf/${name}.pb.cc
            COMMAND
            ${PROTOBUF_PROTOC_EXECUTABLE} --cpp_out=${CMAKE_CURRENT_BINARY_DIR}/protobuf --proto_path=${CMAKE_CURRENT_SOURCE_DIR}/protobuf ${CMAKE_CURRENT_SOURCE_DIR}/protobuf/${name}.proto
            DEPENDS
            ${PROTOBUF_PROTOC_EXECUTABLE}
            ${CMAKE_CURRENT_SOURCE_DIR}/protobuf/${name}.proto
        )
        list(APPEND PROTO ${CMAKE_CURRENT_SOURCE_DIR}/protobuf/${name}.proto)
        list(APPEND PROTO_GEN_FILES ${CMAKE_CURRENT_BINARY_DIR}/protobuf/${name}.pb.h)
        list(APPEND PROTO_GEN_FILES ${CMAKE_CURRENT_BINARY_DIR}/protobuf/${name}.pb.cc)
    endforeach()
endmacro()

include_directories(
    SYSTEM
    ${PROTOBUF_INCLUDE_DIRS}
    ${BMCL_INCLUDE_DIRS}
)

generate_proto_messages(
    Message
    Handshake
    Tm
    Cmd
    Device
    Firmware
    Channel
    System
    Protocol
    IfDev
)

set (MESSAGES
    Channel.h
    Channel.cpp
    Cmd.h
    Cmd.cpp
    Device.h
    Device.cpp
    Firmware.h
    Firmware.cpp
    Message.h
    Message.cpp
    Protocol.h
    Protocol.cpp
    System.h
    System.cpp
    Tm.h
    Tm.cpp
)

set (SOURCES
    ${MESSAGES}
    ${PROTO}
    ${PROTO_GEN_FILES}
    Channel.h
    Channel.cpp
    Cmd.h
    Cmd.cpp
    Device.h
    Device.cpp
    Firmware.h
    Firmware.cpp
    LocalRouter.h
    LocalRouter.cpp
    Message.h
    Message.cpp
    Deaclarations.h
    MessageSender.h
    MessageSender.cpp
    Protocol.h
    Protocol.cpp
    ServiceAbstract.h
    ServiceAbstract.cpp
    System.h
    System.cpp
    Stats.h
    Tm.h
    Tm.cpp
)

source_group("messages" FILES ${MESSAGES})
source_group("messages\\proto" FILES ${PROTO})
source_group("messages\\proto\\gen" FILES ${PROTO_GEN_FILES})
source_group("Source Files" FILES ${SOURCES})

mcc_add_library(mcc-messages-lib
    ${SOURCES}
)

target_link_libraries(mcc-messages-lib
    ${PROTOBUF_LIBRARIES}
    Qt5::Core
    mcc-misc-lib
)

set_target_properties(mcc-messages-lib
    PROPERTIES
    FOLDER "mcc"
)
