project(xz-utils)

cmake_minimum_required(VERSION 2.8)

include(ExternalProject)
externalproject_add(xz-utils
                    SOURCE_DIR ${CMAKE_SOURCE_DIR}
                    UPDATE_COMMAND autoreconf -vif
                    CONFIGURE_COMMAND <SOURCE_DIR>/configure 
                      --prefix=${OUTPUT_DIR}
                      --enable-static
                      --disable-shared
                      --with-pic
                      ${EXTRA_CFLAGS}
                    INSTALL_COMMAND ""
                    BUILD_IN_SOURCE 1)

install(CODE "execute_process(COMMAND make install WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})")
