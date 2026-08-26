# Install script for directory: /session/w683b

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

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/session/w683b/cm/deps/c-build-tools/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/session/w683b/cm/deps/macro-utils-c/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/session/w683b/cm/deps/c-logging/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/session/w683b/cm/deps/ctest/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/session/w683b/cm/deps/c-testrunnerswitcher/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/session/w683b/cm/deps/umock-c/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/session/w683b/cm/testtools/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/session/w683b/cm/tests/cmake_install.cmake")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE STATIC_LIBRARY FILES "/session/w683b/cm/libaziotsharedutil.a")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE STATIC_LIBRARY FILES "/session/w683b/cm/deps/c-logging/v2/libc_logging_v2.a")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE STATIC_LIBRARY FILES "/session/w683b/cm/deps/c-logging/v2/libc_logging_v2_core.a")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/azure_c_shared_utility" TYPE FILE FILES
    "/session/w683b/./inc/azure_c_shared_utility/agenttime.h"
    "/session/w683b/./inc/azure_c_shared_utility/azure_base32.h"
    "/session/w683b/./inc/azure_c_shared_utility/azure_base64.h"
    "/session/w683b/./inc/azure_c_shared_utility/buffer_.h"
    "/session/w683b/./inc/azure_c_shared_utility/constbuffer_array.h"
    "/session/w683b/./inc/azure_c_shared_utility/constbuffer_array_batcher.h"
    "/session/w683b/./inc/azure_c_shared_utility/connection_string_parser.h"
    "/session/w683b/./inc/azure_c_shared_utility/crt_abstractions.h"
    "/session/w683b/./inc/azure_c_shared_utility/constmap.h"
    "/session/w683b/./inc/azure_c_shared_utility/condition.h"
    "/session/w683b/./inc/azure_c_shared_utility/const_defines.h"
    "/session/w683b/inc/azure_c_shared_utility/consolelogger.h"
    "/session/w683b/./inc/azure_c_shared_utility/doublylinkedlist.h"
    "/session/w683b/./inc/azure_c_shared_utility/envvariable.h"
    "/session/w683b/./inc/azure_c_shared_utility/gballoc.h"
    "/session/w683b/./inc/azure_c_shared_utility/gbnetwork.h"
    "/session/w683b/./inc/azure_c_shared_utility/gb_stdio.h"
    "/session/w683b/./inc/azure_c_shared_utility/gb_time.h"
    "/session/w683b/./inc/azure_c_shared_utility/hmac.h"
    "/session/w683b/./inc/azure_c_shared_utility/hmacsha256.h"
    "/session/w683b/./inc/azure_c_shared_utility/http_proxy_io.h"
    "/session/w683b/./inc/azure_c_shared_utility/singlylinkedlist.h"
    "/session/w683b/./inc/azure_c_shared_utility/lock.h"
    "/session/w683b/./inc/azure_c_shared_utility/map.h"
    "/session/w683b/./inc/azure_c_shared_utility/optimize_size.h"
    "/session/w683b/./inc/azure_c_shared_utility/platform.h"
    "/session/w683b/./inc/azure_c_shared_utility/refcount.h"
    "/session/w683b/./inc/azure_c_shared_utility/sastoken.h"
    "/session/w683b/./inc/azure_c_shared_utility/sha-private.h"
    "/session/w683b/./inc/azure_c_shared_utility/shared_util_options.h"
    "/session/w683b/./inc/azure_c_shared_utility/sha.h"
    "/session/w683b/./inc/azure_c_shared_utility/socketio.h"
    "/session/w683b/./inc/azure_c_shared_utility/srw_lock.h"
    "/session/w683b/./inc/azure_c_shared_utility/stdint_ce6.h"
    "/session/w683b/./inc/azure_c_shared_utility/strings.h"
    "/session/w683b/./inc/azure_c_shared_utility/strings_types.h"
    "/session/w683b/./inc/azure_c_shared_utility/string_token.h"
    "/session/w683b/./inc/azure_c_shared_utility/string_tokenizer.h"
    "/session/w683b/./inc/azure_c_shared_utility/string_tokenizer_types.h"
    "/session/w683b/./inc/azure_c_shared_utility/string_utils.h"
    "/session/w683b/./inc/azure_c_shared_utility/tlsio_options.h"
    "/session/w683b/./inc/azure_c_shared_utility/tickcounter.h"
    "/session/w683b/./inc/azure_c_shared_utility/threadapi.h"
    "/session/w683b/./inc/azure_c_shared_utility/xio.h"
    "/session/w683b/./inc/azure_c_shared_utility/uniqueid.h"
    "/session/w683b/./inc/azure_c_shared_utility/uuid.h"
    "/session/w683b/./inc/azure_c_shared_utility/urlencode.h"
    "/session/w683b/./inc/azure_c_shared_utility/vector.h"
    "/session/w683b/./inc/azure_c_shared_utility/vector_types.h"
    "/session/w683b/./inc/azure_c_shared_utility/vector_types_internal.h"
    "/session/w683b/./inc/azure_c_shared_utility/xlogging.h"
    "/session/w683b/./inc/azure_c_shared_utility/constbuffer.h"
    "/session/w683b/./inc/azure_c_shared_utility/tlsio.h"
    "/session/w683b/./inc/azure_c_shared_utility/optionhandler.h"
    "/session/w683b/./inc/azure_c_shared_utility/memory_data.h"
    "/session/w683b/./inc/azure_c_shared_utility/safe_math.h"
    "/session/w683b/./adapters/linux_time.h"
    "/session/w683b/./inc/azure_c_shared_utility/wsio.h"
    "/session/w683b/./inc/azure_c_shared_utility/uws_client.h"
    "/session/w683b/./inc/azure_c_shared_utility/uws_frame_encoder.h"
    "/session/w683b/./inc/azure_c_shared_utility/utf8_checker.h"
    "/session/w683b/./inc/azure_c_shared_utility/ws_url.h"
    "/session/w683b/./inc/azure_c_shared_utility/httpapi.h"
    "/session/w683b/./inc/azure_c_shared_utility/httpapiex.h"
    "/session/w683b/./inc/azure_c_shared_utility/httpapiexsas.h"
    "/session/w683b/./inc/azure_c_shared_utility/httpheaders.h"
    "/session/w683b/./inc/azure_c_shared_utility/tlsio_openssl.h"
    "/session/w683b/./inc/azure_c_shared_utility/x509_openssl.h"
    "/session/w683b/./inc/azure_c_shared_utility/csr_gen.h"
    "/session/w683b/./pal/linux/refcount_os.h"
    )
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/azureiot" TYPE FILE FILES
    "/session/w683b/./testtools/micromock/inc/globalmock.h"
    "/session/w683b/./testtools/micromock/inc/micromock.h"
    "/session/w683b/./testtools/micromock/inc/micromockcallmacros.h"
    "/session/w683b/./testtools/micromock/inc/micromockcharstararenullterminatedstrings.h"
    "/session/w683b/./testtools/micromock/inc/micromockcommon.h"
    "/session/w683b/./testtools/micromock/inc/micromockenumtostring.h"
    "/session/w683b/./testtools/micromock/inc/micromockexception.h"
    "/session/w683b/./testtools/micromock/inc/micromocktestmutex.h"
    "/session/w683b/./testtools/micromock/inc/micromocktestrunnerhooks.h"
    "/session/w683b/./testtools/micromock/inc/mock.h"
    "/session/w683b/./testtools/micromock/inc/mockcallargument.h"
    "/session/w683b/./testtools/micromock/inc/mockcallargumentbase.h"
    "/session/w683b/./testtools/micromock/inc/mockcallcomparer.h"
    "/session/w683b/./testtools/micromock/inc/mockcallrecorder.h"
    "/session/w683b/./testtools/micromock/inc/mockmethodcall.h"
    "/session/w683b/./testtools/micromock/inc/mockmethodcallbase.h"
    "/session/w683b/./testtools/micromock/inc/mockresultvalue.h"
    "/session/w683b/./testtools/micromock/inc/mockvalue.h"
    "/session/w683b/./testtools/micromock/inc/mockvaluebase.h"
    "/session/w683b/./testtools/micromock/inc/nicecallcomparer.h"
    "/session/w683b/./testtools/micromock/inc/runtimemock.h"
    "/session/w683b/./testtools/micromock/inc/stdafx.h"
    "/session/w683b/./testtools/micromock/inc/strictorderedcallcomparer.h"
    "/session/w683b/./testtools/micromock/inc/strictunorderedcallcomparer.h"
    "/session/w683b/./testtools/micromock/inc/targetver.h"
    "/session/w683b/./testtools/micromock/inc/threadsafeglobalmock.h"
    "/session/w683b/./testtools/micromock/inc/timediscretemicromock.h"
    "/session/w683b/./testtools/micromock/inc/timediscretemicromockcallmacros.h"
    "/session/w683b/./testtools/sal/inc/sal.h"
    "/session/w683b/./testtools/sal/inc/no_sal2.h"
    "/session/w683b/./testtools/micromock/inc/globalmock.h"
    "/session/w683b/./testtools/micromock/inc/micromock.h"
    "/session/w683b/./testtools/micromock/inc/micromockcallmacros.h"
    "/session/w683b/./testtools/micromock/inc/micromockcharstararenullterminatedstrings.h"
    "/session/w683b/./testtools/micromock/inc/micromockcommon.h"
    "/session/w683b/./testtools/micromock/inc/micromockenumtostring.h"
    "/session/w683b/./testtools/micromock/inc/micromockexception.h"
    "/session/w683b/./testtools/micromock/inc/micromocktestmutex.h"
    "/session/w683b/./testtools/micromock/inc/micromocktestrunnerhooks.h"
    "/session/w683b/./testtools/micromock/inc/mock.h"
    "/session/w683b/./testtools/micromock/inc/mockcallargument.h"
    "/session/w683b/./testtools/micromock/inc/mockcallargumentbase.h"
    "/session/w683b/./testtools/micromock/inc/mockcallcomparer.h"
    "/session/w683b/./testtools/micromock/inc/mockcallrecorder.h"
    "/session/w683b/./testtools/micromock/inc/mockmethodcall.h"
    "/session/w683b/./testtools/micromock/inc/mockmethodcallbase.h"
    "/session/w683b/./testtools/micromock/inc/mockresultvalue.h"
    "/session/w683b/./testtools/micromock/inc/mockvalue.h"
    "/session/w683b/./testtools/micromock/inc/mockvaluebase.h"
    "/session/w683b/./testtools/micromock/inc/nicecallcomparer.h"
    "/session/w683b/./testtools/micromock/inc/runtimemock.h"
    "/session/w683b/./testtools/micromock/inc/stdafx.h"
    "/session/w683b/./testtools/micromock/inc/strictorderedcallcomparer.h"
    "/session/w683b/./testtools/micromock/inc/strictunorderedcallcomparer.h"
    "/session/w683b/./testtools/micromock/inc/targetver.h"
    "/session/w683b/./testtools/micromock/inc/threadsafeglobalmock.h"
    "/session/w683b/./testtools/micromock/inc/timediscretemicromock.h"
    "/session/w683b/./testtools/micromock/inc/timediscretemicromockcallmacros.h"
    )
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cmake/azure_c_shared_utility/azure_c_shared_utilityTargets.cmake")
    file(DIFFERENT _cmake_export_file_changed FILES
         "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cmake/azure_c_shared_utility/azure_c_shared_utilityTargets.cmake"
         "/session/w683b/cm/CMakeFiles/Export/d38edef68031dc399a64718639337bf5/azure_c_shared_utilityTargets.cmake")
    if(_cmake_export_file_changed)
      file(GLOB _cmake_old_config_files "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cmake/azure_c_shared_utility/azure_c_shared_utilityTargets-*.cmake")
      if(_cmake_old_config_files)
        string(REPLACE ";" ", " _cmake_old_config_files_text "${_cmake_old_config_files}")
        message(STATUS "Old export file \"$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cmake/azure_c_shared_utility/azure_c_shared_utilityTargets.cmake\" will be replaced.  Removing files [${_cmake_old_config_files_text}].")
        unset(_cmake_old_config_files_text)
        file(REMOVE ${_cmake_old_config_files})
      endif()
      unset(_cmake_old_config_files)
    endif()
    unset(_cmake_export_file_changed)
  endif()
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/cmake/azure_c_shared_utility" TYPE FILE FILES "/session/w683b/cm/CMakeFiles/Export/d38edef68031dc399a64718639337bf5/azure_c_shared_utilityTargets.cmake")
  if(CMAKE_INSTALL_CONFIG_NAME MATCHES "^()$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/cmake/azure_c_shared_utility" TYPE FILE FILES "/session/w683b/cm/CMakeFiles/Export/d38edef68031dc399a64718639337bf5/azure_c_shared_utilityTargets-noconfig.cmake")
  endif()
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/cmake/azure_c_shared_utility" TYPE FILE FILES
    "/session/w683b/configs/azure_c_shared_utilityConfig.cmake"
    "/session/w683b/configs/azure_c_shared_utilityFunctions.cmake"
    "/session/w683b/configs/azure_iot_build_rules.cmake"
    "/session/w683b/cm/azure_c_shared_utility/azure_c_shared_utilityConfigVersion.cmake"
    )
endif()

if(CMAKE_INSTALL_COMPONENT)
  set(CMAKE_INSTALL_MANIFEST "install_manifest_${CMAKE_INSTALL_COMPONENT}.txt")
else()
  set(CMAKE_INSTALL_MANIFEST "install_manifest.txt")
endif()

string(REPLACE ";" "\n" CMAKE_INSTALL_MANIFEST_CONTENT
       "${CMAKE_INSTALL_MANIFEST_FILES}")
file(WRITE "/session/w683b/cm/${CMAKE_INSTALL_MANIFEST}"
     "${CMAKE_INSTALL_MANIFEST_CONTENT}")
