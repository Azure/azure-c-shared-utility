# Install script for directory: /session/w683b/deps/c-logging/v2

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "/usr/local")
endif()
string(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
if(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  if(BUILD_TYPE)
    string(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  else()
    set(CMAKE_INSTALL_CONFIG_NAME "")
  endif()
  message(STATUS "Install configuration: \"${CMAKE_INSTALL_CONFIG_NAME}\"")
endif()

# Set the component getting installed.
if(NOT CMAKE_INSTALL_COMPONENT)
  if(COMPONENT)
    message(STATUS "Install component: \"${COMPONENT}\"")
    set(CMAKE_INSTALL_COMPONENT "${COMPONENT}")
  else()
    set(CMAKE_INSTALL_COMPONENT)
  endif()
endif()

# Install shared libraries without execute permission?
if(NOT DEFINED CMAKE_INSTALL_SO_NO_EXE)
  set(CMAKE_INSTALL_SO_NO_EXE "1")
endif()

# Is this installation the result of a crosscompile?
if(NOT DEFINED CMAKE_CROSSCOMPILING)
  set(CMAKE_CROSSCOMPILING "FALSE")
endif()

# Set default install directory permissions.
if(NOT DEFINED CMAKE_OBJDUMP)
  set(CMAKE_OBJDUMP "/home/agent/.local/usr/bin/objdump")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE STATIC_LIBRARY FILES "/session/w683b/cm/deps/c-logging/v2/libc_logging_v2.a")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/c_logging/v2/c_logging" TYPE FILE FILES
    "/session/w683b/deps/c-logging/v2/./inc/c_logging/logger.h"
    "/session/w683b/deps/c-logging/v2/./inc/c_logging/logger_v1_v2.h"
    "/session/w683b/deps/c-logging/v2/./inc/c_logging/log_context.h"
    "/session/w683b/deps/c-logging/v2/./inc/c_logging/log_context_property_type.h"
    "/session/w683b/deps/c-logging/v2/./inc/c_logging/log_context_property_type_if.h"
    "/session/w683b/deps/c-logging/v2/./inc/c_logging/log_context_property_basic_types.h"
    "/session/w683b/deps/c-logging/v2/./inc/c_logging/log_context_property_bool_type.h"
    "/session/w683b/deps/c-logging/v2/./inc/c_logging/log_context_property_to_string.h"
    "/session/w683b/deps/c-logging/v2/./inc/c_logging/log_context_property_type_ascii_char_ptr.h"
    "/session/w683b/deps/c-logging/v2/./inc/c_logging/log_context_property_type_struct.h"
    "/session/w683b/deps/c-logging/v2/./inc/c_logging/log_context_property_value_pair.h"
    "/session/w683b/deps/c-logging/v2/./inc/c_logging/log_context_property_type_wchar_t_ptr.h"
    "/session/w683b/deps/c-logging/v2/./inc/c_logging/log_errno.h"
    "/session/w683b/deps/c-logging/v2/./inc/c_logging/log_internal_error.h"
    "/session/w683b/deps/c-logging/v2/./inc/c_logging/log_level.h"
    "/session/w683b/deps/c-logging/v2/./inc/c_logging/log_sink_if.h"
    "/session/w683b/deps/c-logging/v2/./inc/c_logging/log_sink_console.h"
    "/session/w683b/deps/c-logging/v2/./inc/c_logging/log_sink_callback.h"
    "/session/w683b/deps/c-logging/v2/./inc/c_logging/logging_stacktrace.h"
    )
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/session/w683b/cm/deps/c-logging/v2/tests/cmake_install.cmake")
endif()

