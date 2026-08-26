# Install script for directory: /session/w683b/deps/umock-c

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

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified")
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE STATIC_LIBRARY FILES "/session/w683b/cm/deps/umock-c/libumock_c.a")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/umock_c" TYPE FILE FILES
    "/session/w683b/deps/umock-c/./inc/umock_c/umock_c.h"
    "/session/w683b/deps/umock-c/./inc/umock_c/umock_c_internal.h"
    "/session/w683b/deps/umock-c/./inc/umock_c/umock_c_negative_tests.h"
    "/session/w683b/deps/umock-c/./inc/umock_c/umock_c_prod.h"
    "/session/w683b/deps/umock-c/./inc/umock_c/umock_lock_factory.h"
    "/session/w683b/deps/umock-c/./inc/umock_c/umock_lock_factory_default.h"
    "/session/w683b/deps/umock-c/./inc/umock_c/umock_lock_if.h"
    "/session/w683b/deps/umock-c/./inc/umock_c/umock_log.h"
    "/session/w683b/deps/umock-c/./inc/umock_c/umockalloc.h"
    "/session/w683b/deps/umock-c/./inc/umock_c/umockautoignoreargs.h"
    "/session/w683b/deps/umock-c/./inc/umock_c/umockcall.h"
    "/session/w683b/deps/umock-c/./inc/umock_c/umockcallpairs.h"
    "/session/w683b/deps/umock-c/./inc/umock_c/umockcallrecorder.h"
    "/session/w683b/deps/umock-c/./inc/umock_c/umockstring.h"
    "/session/w683b/deps/umock-c/./inc/umock_c/umocktypename.h"
    "/session/w683b/deps/umock-c/./inc/umock_c/umocktypes.h"
    "/session/w683b/deps/umock-c/./inc/umock_c/umocktypes_bool.h"
    "/session/w683b/deps/umock-c/./inc/umock_c/umocktypes_c.h"
    "/session/w683b/deps/umock-c/./inc/umock_c/umocktypes_stdint.h"
    "/session/w683b/deps/umock-c/./inc/umock_c/umocktypes_charptr.h"
    "/session/w683b/deps/umock-c/./inc/umock_c/umocktypes_struct.h"
    "/session/w683b/deps/umock-c/./inc/umock_c/umocktypes_wcharptr.h"
    )
endif()

