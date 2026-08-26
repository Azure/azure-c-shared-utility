#----------------------------------------------------------------
# Generated CMake target import file.
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "macro_utils_c" for configuration ""
set_property(TARGET macro_utils_c APPEND PROPERTY IMPORTED_CONFIGURATIONS NOCONFIG)
set_target_properties(macro_utils_c PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_NOCONFIG "C"
  IMPORTED_LOCATION_NOCONFIG "${_IMPORT_PREFIX}/lib/libmacro_utils_c.a"
  )

list(APPEND _cmake_import_check_targets macro_utils_c )
list(APPEND _cmake_import_check_files_for_macro_utils_c "${_IMPORT_PREFIX}/lib/libmacro_utils_c.a" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
