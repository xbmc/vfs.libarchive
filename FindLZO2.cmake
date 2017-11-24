#.rst:
# FindLZO2
# --------
# Finds the LZO2 library
#
# This will will define the following variables::
#
# LZO2_FOUND - system has LZO2
# LZO2_INCLUDE_DIRS - the LZO2 include directory
# LZO2_LIBRARIES - the LZO2 libraries
#
# and the following imported targets::
#
#   LZO2::LZO2 - The LZO2 library

if(PKG_CONFIG_FOUND)
  pkg_check_modules(PC_LZO2 lzo2 QUIET)
endif()

find_path(LZO2_INCLUDE_DIR NAMES lzo/lzo2a.h
                           PATHS ${PC_LZO2_INCLUDEDIR})
find_library(LZO2_LIBRARY NAMES lzo2
                          PATHS ${PC_LZO2_LIBDIR})

set(LZO2_VERSION ${PC_LZO2_VERSION})

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(LZO2
                                  REQUIRED_VARS LZO2_LIBRARY LZO2_INCLUDE_DIR
                                  VERSION_VAR LZO2_VERSION)

if(LZO2_FOUND)
  set(LZO2_LIBRARIES ${LZO2_LIBRARY})

  if(NOT TARGET LZO2::LZO2)
    add_library(LZO2::LZO2 UNKNOWN IMPORTED)
    set_target_properties(LZO2::LZO2 PROPERTIES
                                     IMPORTED_LOCATION "${LZO2_LIBRARY}")
  endif()
endif()

mark_as_advanced(LZO2_INCLUDE_DIR LZO2_LIBRARY)
