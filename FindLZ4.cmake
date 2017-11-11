#.rst:
# FindLZ4
# --------
# Finds the LZ4 library
#
# This will will define the following variables::
#
# LZ4_FOUND - system has lz4
# LZ4_INCLUDE_DIRS - the lz4 include directory
# LZ4_LIBRARIES - the lz4 libraries
#
# and the following imported targets::
#
#   LZ4::LZ4- The LZ4 library

if(PKG_CONFIG_FOUND)
  pkg_check_modules(PC_LZ4 liblz4 QUIET)
endif()

find_path(LZ4_INCLUDE_DIR NAMES lz4.h
                           PATHS ${PC_LZ4_INCLUDEDIR})
find_library(LZ4_LIBRARY NAMES lz4
                          PATHS ${PC_LZ4_LIBDIR})

set(LZ4_VERSION ${PC_LZ4_VERSION})

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(LZ4
                                  REQUIRED_VARS LZ4_LIBRARY LZ4_INCLUDE_DIR
                                  VERSION_VAR LZ4_VERSION)

if(LZ4_FOUND)
  set(LZ4_LIBRARIES ${LZ4_LIBRARY})

  if(NOT TARGET LZ4::LZ4)
    add_library(LZ4::LZ4 UNKNOWN IMPORTED)
    set_target_properties(LZ4::LZ4 PROPERTIES
                                   IMPORTED_LOCATION "${LZ4_LIBRARY}")
  endif()
endif()

mark_as_advanced(LZ4_INCLUDE_DIR LZ4_LIBRARY)
