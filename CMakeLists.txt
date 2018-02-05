project(vfs.libarchive)

cmake_minimum_required(VERSION 2.6)

list(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR})

enable_language(CXX)

find_package(Kodi REQUIRED)
find_package(LibArchive REQUIRED)
find_package(LibLZMA REQUIRED)
find_package(BZip2 REQUIRED)
find_package(ZLIB REQUIRED)
find_package(LZ4 REQUIRED)
find_package(LZO2 REQUIRED)
find_package(OpenSSL REQUIRED)

include_directories(${KODI_INCLUDE_DIR}/..
                    ${LibArchive_INCLUDE_DIRS}
                    ${LIBLZMA_INCLUDE_DIRS}
                    ${BZIP2_INCLUDE_DIR}
                    ${ZLIB_INCLUDE_DIRS}
                    ${LZ4_INCLUDE_DIRS}
                    ${LZO2_INCLUDE_DIRS}
                    ${OPENSSL_INCLUDE_DIR})

set(ARCHIVE_SOURCES src/ArchiveFile.cpp)

set(DEPLIBS ${LibArchive_LIBRARIES}
            ${LIBLZMA_LIBRARIES}
            ${BZIP2_LIBRARIES}
            ${ZLIB_LIBRARIES}
            ${LZ4_LIBRARIES}
            ${LZO2_LIBRARIES}
            ${OPENSSL_LIBRARIES})

add_definitions(-DLIBARCHIVE_STATIC)

build_addon(vfs.libarchive ARCHIVE DEPLIBS)
include(CPack)
