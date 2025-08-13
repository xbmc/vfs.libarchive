cmake_minimum_required(VERSION 3.5)
project(vfs.libarchive)

list(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR})

find_package(Kodi REQUIRED)
find_package(LibArchive REQUIRED)
find_package(LibLZMA)
find_package(BZip2 REQUIRED)
find_package(ZLIB REQUIRED)
find_package(LZ4 REQUIRED)
find_package(LZO2 REQUIRED)
find_package(OpenSSL REQUIRED)

include_directories(${KODI_INCLUDE_DIR}/..
                    ${LibArchive_INCLUDE_DIRS}
                    ${BZIP2_INCLUDE_DIR}
                    ${ZLIB_INCLUDE_DIRS}
                    ${LZ4_INCLUDE_DIRS}
                    ${LZO2_INCLUDE_DIRS}
                    ${OPENSSL_INCLUDE_DIR})

set(ARCHIVE_SOURCES src/ArchiveFile.cpp)

set(DEPLIBS ${LibArchive_LIBRARIES}
            ${BZIP2_LIBRARIES}
            ${OPENSSL_LIBRARIES}
            ${ZLIB_LIBRARIES}
            ${LZ4_LIBRARIES}
            ${LZO2_LIBRARIES})

if(LibLZMA_FOUND)
  include_directories(${LIBLZMA_INCLUDE_DIRS})
  list(APPEND DEPLIBS ${LIBLZMA_LIBRARIES})
endif()

add_definitions(-DLIBARCHIVE_STATIC -D_CRT_NONSTDC_NO_DEPRECATE)

build_addon(vfs.libarchive ARCHIVE DEPLIBS)
include(CPack)
