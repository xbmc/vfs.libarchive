cmake_minimum_required(VERSION 3.10)
project(xz-utils)

include(ExternalProject)
externalproject_add(xz-utils
                    SOURCE_DIR ${CMAKE_SOURCE_DIR}
                    CONFIGURE_COMMAND <SOURCE_DIR>/configure
                      --prefix=${OUTPUT_DIR}
                      --enable-static
                      --disable-shared
                      --with-pic
                      --disable-doc
                      --disable-lzmadec
                      --disable-lzmainfo
                      --disable-scripts
                      --disable-xz
                      --disable-xzdec
                      ${EXTRA_CFLAGS}
                    INSTALL_COMMAND ""
                    BUILD_IN_SOURCE 1)

install(CODE "execute_process(COMMAND make install WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})")
