project(bzip2)

cmake_minimum_required(VERSION 2.8)

if(NOT WIN32)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D_FILE_OFFSET_BITS=64")
endif()

set(SOURCES blocksort.c
            huffman.c
            crctable.c
            randtable.c
            compress.c
            decompress.c
            bzlib.c)

add_library(bz2 ${SOURCES})

install(TARGETS bz2 DESTINATION lib)
install(FILES bzlib.h DESTINATION include)
