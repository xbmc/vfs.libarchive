project(vfs.libarchive)

cmake_minimum_required(VERSION 2.6)

list(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR})

enable_language(CXX)

find_package(Kodi REQUIRED)
find_package(LibArchive REQUIRED)

include_directories(${KODI_INCLUDE_DIR}/..
                    ${LibArchive_INCLUDE_DIRS})

set(ARCHIVE_SOURCES src/ArchiveFile.cpp)

set(DEPLIBS ${LibArchive_LIBRARIES})

build_addon(vfs.libarchive ARCHIVE DEPLIBS)
include(CPack)
