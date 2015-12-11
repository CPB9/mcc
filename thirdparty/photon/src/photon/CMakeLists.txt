if(NOT MSVC)
    add_definitions(-std=c99)
endif()

add_library(photon
    Reader.c
    Reader.h
    Result.h
    RingBuffer.c
    RingBuffer.h
    Writer.c
    Writer.h
    Decoder.c
    Decoder.h
    Encoder.c
    Encoder.h
    Parser.c
    Parser.h
    Exchange.c
    Exchange.h
)
