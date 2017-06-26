#Copyright (c) Microsoft. All rights reserved.
#Licensed under the MIT license. See LICENSE file in the project root for full license information.

cmake_minimum_required(VERSION 2.8.11)

if (TARGET aziotsharedutil)
    RETURN()
endif()

project(azure_c_shared_utility)

set(C_SHARED_VERSION 1.0.37)

if(POLICY CMP0042)
    cmake_policy(SET CMP0042 NEW)
endif()

option(run_valgrind "set run_valgrind to ON if tests are to be run under valgrind/helgrind/drd. Default is OFF" OFF)

#
#making a global variable to know if we are on linux, windows, or macosx.
if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    set(WINDOWS TRUE)
elseif(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    set(LINUX TRUE)
elseif(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    set(MACOSX TRUE)
endif()

include(CTest)
include(CheckIncludeFiles)

#these are the include folders
#the following "set" statetement exports across the project a global variable called COMMON_INC_FOLDER that expands to whatever needs to included when using COMMON library
set(SHARED_UTIL_INC_FOLDER ${CMAKE_CURRENT_LIST_DIR}/inc CACHE INTERNAL "this is what needs to be included if using sharedLib lib" FORCE)
set(SHARED_UTIL_SRC_FOLDER ${CMAKE_CURRENT_LIST_DIR}/src CACHE INTERNAL "this is what needs to be included when doing include sources" FORCE)
set(SHARED_UTIL_ADAPTER_FOLDER "${CMAKE_CURRENT_LIST_DIR}/adapters" CACHE INTERNAL "this is where the adapters live" FORCE)

include_directories(${SHARED_UTIL_INC_FOLDER})

CHECK_INCLUDE_FILES(stdint.h HAVE_STDINT_H)
CHECK_INCLUDE_FILES(stdbool.h HAVE_STDBOOL_H)

if ((NOT HAVE_STDINT_H) OR (NOT HAVE_STDBOOL_H))
    include_directories(${SHARED_UTIL_INC_FOLDER}/azure_c_shared_utility/windowsce)
endif()

#the following variables are project-wide and can be used with cmake-gui
option(run_unittests "set run_unittests to ON to run unittests (default is OFF)" OFF)
option(skip_samples "set skip_samples to ON to skip building samples (default is OFF)[if possible, they are always built]" OFF)
option(use_http "set use_http to ON if http is to be used, set to OFF to not use http" ON)
option(use_condition "set use_condition to ON if the condition module and its adapters should be enabled" ON)
option(use_wsio "set use_wsio to ON to build WebSockets support (default is ON)" ON)
option(nuget_e2e_tests "set nuget_e2e_tests to ON to generate e2e tests to run with nuget packages (default is OFF)" OFF)
option(use_installed_dependencies "set use_installed_dependencies to ON to use installed packages instead of building dependencies from submodules" OFF)
option(use_default_uuid "set use_default_uuid to ON to use the out of the box UUID that comes with the SDK rather than platform specific implementations" OFF)
option(run_e2e_tests "set run_e2e_tests to ON to run e2e tests (default is OFF). Chsare dutility does not have any e2e tests, but the option needs to exist to evaluate in IF statements" OFF)
option(use_builtin_httpapi "set use_builtin_httpapi to ON to use the built-in httpapi_compact that comes with C shared utility (default is OFF)" OFF)
option(use_cppunittest "set use_cppunittest to ON to build CppUnitTest tests on Windows (default is ON)" ON)

if(WIN32)
    option(use_schannel "set use_schannel to ON if schannel is to be used, set to OFF to not use schannel" ON)
    option(use_openssl "set use_openssl to ON if openssl is to be used, set to OFF to not use openssl" OFF)
    option(use_wolfssl "set use_wolfssl to ON if wolfssl is to be used, set to OFF to not use wolfssl" OFF)
else()
    option(use_schannel "set use_schannel to ON if schannel is to be used, set to OFF to not use schannel" OFF)
    option(use_openssl "set use_openssl to ON if openssl is to be used, set to OFF to not use openssl" ON)
    option(use_wolfssl "set use_wolfssl to ON if wolfssl is to be used, set to OFF to not use wolfssl" OFF)
endif()
option(use_socketio "set use_socketio to ON if socketio is to be included in the library, set to OFF if a different implementation will be provided" ON)
option(use_cyclonessl "set use_cyclonessl to ON if cyclonessl is to be used, set to OFF to not use cyclonessl" OFF)

option(compileOption_C "passes a string to the command line of the C compiler" OFF)
option(compileOption_CXX "passes a string to the command line of the C++ compiler" OFF)
option(build_as_dynamic "build the C Shared libaries as shared"  OFF)

if(${use_cyclonessl} AND WIN32)
    set(cyclonessl_c_files
        $ENV{CycloneSSLDir}/cyclone_ssl/ssl_common.c
        $ENV{CycloneSSLDir}/cyclone_ssl/tls.c
        $ENV{CycloneSSLDir}/cyclone_ssl/tls_cache.c
        $ENV{CycloneSSLDir}/cyclone_ssl/tls_cipher_suites.c
        $ENV{CycloneSSLDir}/cyclone_ssl/tls_client.c
        $ENV{CycloneSSLDir}/cyclone_ssl/tls_common.c
        $ENV{CycloneSSLDir}/cyclone_ssl/tls_io.c
        $ENV{CycloneSSLDir}/cyclone_ssl/tls_misc.c
        $ENV{CycloneSSLDir}/cyclone_ssl/tls_record.c
        $ENV{CycloneSSLDir}/cyclone_ssl/tls_server.c
        $ENV{CycloneSSLDir}/common/date_time.c
        $ENV{CycloneSSLDir}/common/debug.c
        $ENV{CycloneSSLDir}/common/endian.c
        $ENV{CycloneSSLDir}/common/os_port_windows.c
        $ENV{CycloneSSLDir}/cyclone_crypto/aes.c
        $ENV{CycloneSSLDir}/cyclone_crypto/aria.c
        $ENV{CycloneSSLDir}/cyclone_crypto/asn1.c
        $ENV{CycloneSSLDir}/cyclone_crypto/base64.c
        $ENV{CycloneSSLDir}/cyclone_crypto/camellia.c
        $ENV{CycloneSSLDir}/cyclone_crypto/chacha.c
        $ENV{CycloneSSLDir}/cyclone_crypto/chacha20_poly1305.c
        $ENV{CycloneSSLDir}/cyclone_crypto/cipher_mode_cbc.c
        $ENV{CycloneSSLDir}/cyclone_crypto/cipher_mode_ccm.c
        $ENV{CycloneSSLDir}/cyclone_crypto/cipher_mode_gcm.c
        $ENV{CycloneSSLDir}/cyclone_crypto/des.c
        $ENV{CycloneSSLDir}/cyclone_crypto/des3.c
        $ENV{CycloneSSLDir}/cyclone_crypto/dh.c
        $ENV{CycloneSSLDir}/cyclone_crypto/dsa.c
        $ENV{CycloneSSLDir}/cyclone_crypto/ec.c
        $ENV{CycloneSSLDir}/cyclone_crypto/ec_curves.c
        $ENV{CycloneSSLDir}/cyclone_crypto/ecdh.c
        $ENV{CycloneSSLDir}/cyclone_crypto/ecdsa.c
        $ENV{CycloneSSLDir}/cyclone_crypto/hmac.c
        $ENV{CycloneSSLDir}/cyclone_crypto/idea.c
        $ENV{CycloneSSLDir}/cyclone_crypto/md5.c
        $ENV{CycloneSSLDir}/cyclone_crypto/mpi.c
        $ENV{CycloneSSLDir}/cyclone_crypto/oid.c
        $ENV{CycloneSSLDir}/cyclone_crypto/pem.c
        $ENV{CycloneSSLDir}/cyclone_crypto/poly1305.c
        $ENV{CycloneSSLDir}/cyclone_crypto/rc4.c
        $ENV{CycloneSSLDir}/cyclone_crypto/rsa.c
        $ENV{CycloneSSLDir}/cyclone_crypto/seed.c
        $ENV{CycloneSSLDir}/cyclone_crypto/sha1.c
        $ENV{CycloneSSLDir}/cyclone_crypto/sha224.c
        $ENV{CycloneSSLDir}/cyclone_crypto/sha256.c
        $ENV{CycloneSSLDir}/cyclone_crypto/sha384.c
        $ENV{CycloneSSLDir}/cyclone_crypto/sha512.c
        $ENV{CycloneSSLDir}/cyclone_crypto/x509.c
        $ENV{CycloneSSLDir}/cyclone_crypto/yarrow.c
    )
    
    include_directories($ENV{CycloneSSLDir}/common/)
    include_directories($ENV{CycloneSSLDir}/cyclone_ssl/)
    include_directories($ENV{CycloneSSLDir}/cyclone_crypto/)
    include_directories($ENV{CycloneSSLDir}/demo/x86/ssl_client_demo/src/)

    set_source_files_properties(${cyclonessl_c_files} PROPERTIES COMPILE_FLAGS "/wd4200 /wd4201 /wd4244 /wd4100 /wd4267 /wd4701 /wd4703 /wd4389 /wd4005")
    set_source_files_properties(./src/tlsio_cyclonessl.c ./src/tlsio_cyclonessl_socket_bsd.c PROPERTIES COMPILE_FLAGS "/wd4200 /wd4201 /wd4244 /wd4100 /wd4267 /wd4701 /wd4703 /wd4389 /wd4005 /wd4996")
    add_library(cyclonessl ${cyclonessl_c_files})

endif()

if(${use_openssl})
    if("${OPENSSL_ROOT_DIR}" STREQUAL "" AND NOT ("$ENV{OpenSSLDir}" STREQUAL ""))
        set(OPENSSL_ROOT_DIR $ENV{OpenSSLDir} CACHE PATH "")
    endif()

    find_package(OpenSSL REQUIRED)
    include_directories(${OPENSSL_INCLUDE_DIR})
endif()

# Start of variables used during install
set (LIB_INSTALL_DIR lib CACHE PATH "Library object file directory")

#Use solution folders.
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

if(MSVC)
    if (WINCE) # Be lax with WEC 2013 compiler
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W3")
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /W3")
        # WEC 2013 uses older VS compiler. Build some files as C++ files to resolve C99 related compile issues
        SET_SOURCE_FILES_PROPERTIES(src/consolelogger.c src/xlogging.c src/map.c adapters/uniqueid_win32.c src/tlsio_schannel.c src/x509_schannel.c PROPERTIES LANGUAGE CXX)
        if (${use_httpapi_compact})
            SET_SOURCE_FILES_PROPERTIES(adapters/httpapi_compact.c PROPERTIES LANGUAGE CXX)
        else()
            SET_SOURCE_FILES_PROPERTIES(adapters/httpapi_wince.c PROPERTIES LANGUAGE CXX)
        endif()
        add_definitions(-DWIN32) #WEC 2013
    ELSE()

        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4")
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /W4")

        if(${use_openssl})
            set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DUSE_OPENSSL")
            set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DUSE_OPENSSL")
        endif()
        if(${use_cyclonessl})
            set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DUSE_CYCLONESSL")
            set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DUSE_CYCLONESSL")
        endif()

    endif()
elseif(UNIX) #LINUX OR APPLE
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC -Werror")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fPIC -Werror")
    if(NOT (IN_OPENWRT OR APPLE))
        set (CMAKE_C_FLAGS "-D_POSIX_C_SOURCE=200112L ${CMAKE_C_FLAGS}")
    endif()
endif()

enable_testing()

include(CheckSymbolExists)
function(detect_architecture symbol arch)
    if (NOT DEFINED ARCHITECTURE OR ARCHITECTURE STREQUAL "")
        set(CMAKE_REQUIRED_QUIET 1)
        check_symbol_exists("${symbol}" "" ARCHITECTURE_${arch})
        unset(CMAKE_REQUIRED_QUIET)

        # The output variable needs to be unique across invocations otherwise
        # CMake's crazy scope rules will keep it defined
        if (ARCHITECTURE_${arch})
            set(ARCHITECTURE "${arch}" PARENT_SCOPE)
            set(ARCHITECTURE_${arch} 1 PARENT_SCOPE)
            add_definitions(-DARCHITECTURE_${arch}=1)
        endif()
    endif()
endfunction()
if (MSVC)
    detect_architecture("_M_AMD64" x86_64)
    detect_architecture("_M_IX86" x86)
    detect_architecture("_M_ARM" ARM)
else()
    detect_architecture("__x86_64__" x86_64)
    detect_architecture("__i386__" x86)
    detect_architecture("__arm__" ARM)
endif()
if (NOT DEFINED ARCHITECTURE OR ARCHITECTURE STREQUAL "")
    set(ARCHITECTURE "GENERIC")
endif()
message(STATUS "Shared_Util architecture: ${ARCHITECTURE}")

#if any compiler has a command line switch called "OFF" then it will need special care
if(NOT "${compileOption_C}" STREQUAL "OFF")
    set(CMAKE_C_FLAGS "${compileOption_C} ${CMAKE_C_FLAGS}")
endif()

if(NOT "${compileOption_CXX}" STREQUAL "OFF")
    set(CMAKE_CXX_FLAGS "${compileOption_CXX} ${CMAKE_CXX_FLAGS}")
endif()

#this project uses several other projects that are build not by these CMakeFiles
#this project also targets several OSes

#this function takes care of three things:
#1. copying some shared libraries(.dll or .so) to the location of the output executable

include(CheckCXXCompilerFlag)
CHECK_CXX_COMPILER_FLAG("-std=c++11" CXX_FLAG_CXX11)

macro(compileAsC99)
  if (CMAKE_VERSION VERSION_LESS "3.1")
    if (CMAKE_C_COMPILER_ID STREQUAL "GNU")
      set (CMAKE_C_FLAGS "--std=c99 ${CMAKE_C_FLAGS}")
      if (CXX_FLAG_CXX11)
        set (CMAKE_CXX_FLAGS "--std=c++11 ${CMAKE_CXX_FLAGS}")
      else()
        set (CMAKE_CXX_FLAGS "--std=c++0x ${CMAKE_CXX_FLAGS}")
      endif()
    endif()
  else()
    set (CMAKE_C_STANDARD 99)
    set (CMAKE_CXX_STANDARD 11)
  endif()
endmacro(compileAsC99)

macro(compileAsC11)
  if (CXX_FLAG_CXX11)
    if (CMAKE_VERSION VERSION_LESS "3.1")
      if (CMAKE_C_COMPILER_ID STREQUAL "GNU")
        set (CMAKE_C_FLAGS "--std=c11 ${CMAKE_C_FLAGS}")
        set (CMAKE_C_FLAGS "-D_POSIX_C_SOURCE=200112L ${CMAKE_C_FLAGS}")
        set (CMAKE_CXX_FLAGS "--std=c++11 ${CMAKE_CXX_FLAGS}")
      endif()
    else()
      set (CMAKE_C_STANDARD 11)
      set (CMAKE_CXX_STANDARD 11)
    endif()
  else()
    if (CMAKE_C_COMPILER_ID STREQUAL "GNU")
        set (CMAKE_C_FLAGS "--std=c99 ${CMAKE_C_FLAGS}")
        set (CMAKE_CXX_FLAGS "--std=c++0x ${CMAKE_CXX_FLAGS}")
    else()
      set (CMAKE_C_STANDARD 11)
      set (CMAKE_CXX_STANDARD 11)
    endif()
  endif()
endmacro(compileAsC11)

function(add_files_to_install filesToBeInstalled)
    set(INSTALL_H_FILES ${INSTALL_H_FILES} ${filesToBeInstalled} CACHE INTERNAL "Files that will be installed on the system")
endfunction()

include("${CMAKE_CURRENT_LIST_DIR}/configs/azure_c_shared_utilityFunctions.cmake")

#Setup the platform files
set_platform_files(${CMAKE_CURRENT_LIST_DIR})

include_directories(${UMOCK_C_INC_FOLDER})

compileAsC99()

#these are the C source files
set(source_c_files
./src/base64.c
./src/buffer.c
./src/connection_string_parser.c
./src/constbuffer.c
./src/consolelogger.c
./src/crt_abstractions.c
./src/constmap.c
./src/doublylinkedlist.c
./src/gballoc.c
./src/gb_stdio.c
./src/gb_time.c
./src/gb_rand.c
./src/hmac.c
./src/hmacsha256.c
./src/http_proxy_io.c
./src/xio.c
./src/singlylinkedlist.c
./src/map.c
./src/sastoken.c
./src/sha1.c
./src/sha224.c
./src/sha384-512.c
./src/strings.c
./src/string_tokenizer.c
./src/urlencode.c
./src/usha.c
./src/vector.c
./src/xlogging.c
./src/optionhandler.c
./adapters/agenttime.c
${CONDITION_C_FILE}
${LOCK_C_FILE}
${PLATFORM_C_FILE}
${SOCKETIO_C_FILE}
${TICKCOUTER_C_FILE}
${THREAD_C_FILE}
${UNIQUEID_C_FILE}
)

if(UNIX) #LINUX OR APPLE
    set(source_c_files ${source_c_files}
        ./adapters/linux_time.c
    )
endif()

if(${use_http})
    set(source_c_files ${source_c_files}
        ./src/httpapiex.c
        ./src/httpapiexsas.c
        ./src/httpheaders.c
        ${HTTP_C_FILE}
    )
endif()

if(${use_schannel})
    set(source_c_files ${source_c_files}
        ./src/tlsio_schannel.c
        ./src/x509_schannel.c
    )
endif()
if(${use_wolfssl})
    set(source_c_files ${source_c_files}
        ./src/tlsio_wolfssl.c
    )
endif()
if(${use_openssl})
    set(source_c_files ${source_c_files}
        ./src/tlsio_openssl.c
        ./src/x509_openssl.c
    )
endif()
if(${use_cyclonessl} AND WIN32)
    set(source_c_files ${source_c_files}
        ./src/tlsio_cyclonessl.c
        ./src/tlsio_cyclonessl_socket_bsd.c
        )
endif()

#these are the C headers
set(source_h_files
./inc/azure_c_shared_utility/agenttime.h
./inc/azure_c_shared_utility/base64.h
./inc/azure_c_shared_utility/buffer_.h
./inc/azure_c_shared_utility/connection_string_parser.h
./inc/azure_c_shared_utility/crt_abstractions.h
./inc/azure_c_shared_utility/constmap.h
./inc/azure_c_shared_utility/condition.h
./inc/azure_c_shared_utility/consolelogger.h
./inc/azure_c_shared_utility/doublylinkedlist.h
./inc/azure_c_shared_utility/gballoc.h
./inc/azure_c_shared_utility/gb_stdio.h
./inc/azure_c_shared_utility/gb_time.h
./inc/azure_c_shared_utility/gb_rand.h
./inc/azure_c_shared_utility/hmac.h
./inc/azure_c_shared_utility/hmacsha256.h
./inc/azure_c_shared_utility/http_proxy_io.h
./inc/azure_c_shared_utility/singlylinkedlist.h
./inc/azure_c_shared_utility/lock.h
./inc/azure_c_shared_utility/macro_utils.h
./inc/azure_c_shared_utility/map.h
./inc/azure_c_shared_utility/optimize_size.h
./inc/azure_c_shared_utility/platform.h
./inc/azure_c_shared_utility/refcount.h
./inc/azure_c_shared_utility/sastoken.h
./inc/azure_c_shared_utility/sha-private.h
./inc/azure_c_shared_utility/shared_util_options.h
./inc/azure_c_shared_utility/sha.h
./inc/azure_c_shared_utility/socketio.h
./inc/azure_c_shared_utility/stdint_ce6.h
./inc/azure_c_shared_utility/strings.h
./inc/azure_c_shared_utility/strings_types.h
./inc/azure_c_shared_utility/string_tokenizer.h
./inc/azure_c_shared_utility/string_tokenizer_types.h
./inc/azure_c_shared_utility/tickcounter.h
./inc/azure_c_shared_utility/threadapi.h
./inc/azure_c_shared_utility/xio.h
./inc/azure_c_shared_utility/umock_c_prod.h
./inc/azure_c_shared_utility/uniqueid.h
./inc/azure_c_shared_utility/urlencode.h
./inc/azure_c_shared_utility/vector.h
./inc/azure_c_shared_utility/vector_types.h
./inc/azure_c_shared_utility/vector_types_internal.h
./inc/azure_c_shared_utility/xlogging.h
./inc/azure_c_shared_utility/constbuffer.h
./inc/azure_c_shared_utility/tlsio.h
./inc/azure_c_shared_utility/optionhandler.h
)

if(UNIX) #LINUX OR APPLE
    set(source_h_files ${source_h_files}
        ./adapters/linux_time.h
    )
endif()

if(${use_wsio})
    set(source_h_files ${source_h_files}
        ./inc/azure_c_shared_utility/wsio.h
        ./inc/azure_c_shared_utility/uws_client.h
        ./inc/azure_c_shared_utility/uws_frame_encoder.h
        ./inc/azure_c_shared_utility/utf8_checker.h
    )
    set(source_c_files ${source_c_files}
        ./src/wsio.c
        ./src/uws_client.c
        ./src/uws_frame_encoder.c
        ./src/utf8_checker.c
    )
endif()

if(${use_http})
    set(source_h_files ${source_h_files}
        ./inc/azure_c_shared_utility/httpapi.h
        ./inc/azure_c_shared_utility/httpapiex.h
        ./inc/azure_c_shared_utility/httpapiexsas.h
        ./inc/azure_c_shared_utility/httpheaders.h
    )
endif()

if(${use_schannel})
    set(source_h_files ${source_h_files}
        ./inc/azure_c_shared_utility/tlsio_schannel.h
        ./inc/azure_c_shared_utility/x509_schannel.h
        )
endif()
if(${use_wolfssl})
    set(source_h_files ${source_h_files}
        ./inc/azure_c_shared_utility/tlsio_wolfssl.h
    )
    if(WIN32)
        include_directories($ENV{WolfSSLDir})
    endif()
endif()
if(${use_openssl})
    set(source_h_files ${source_h_files}
        ./inc/azure_c_shared_utility/tlsio_openssl.h
        ./inc/azure_c_shared_utility/x509_openssl.h
        )
endif()
if(${use_cyclonessl} AND WIN32)
    set(source_h_files ${source_h_files}
        ./inc/azure_c_shared_utility/tlsio_cyclonessl.h
        ./inc/azure_c_shared_utility/tlsio_cyclonessl_socket.h
        )
endif()

IF(WIN32)
    #windows needs this define
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
    IF(WINCE)
    # Don't treat warning as errors for WEC 2013. WEC 2013 uses older compiler version
    add_definitions(/WX-)
    ELSE()
    # Make warning as error
    add_definitions(/WX)
    ENDIF()
ELSE()
    # Make warning as error
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Werror")
ENDIF(WIN32)

#this is the product (a library)
add_library(aziotsharedutil ${source_c_files} ${source_h_files})

#make sure we can link as dll/so
add_library(aziotsharedutil_dll SHARED ./src/aziotsharedutil.def ${source_c_files} ${source_h_files})
set_target_properties(aziotsharedutil_dll PROPERTIES OUTPUT_NAME "aziotsharedutil")

if(WIN32)

endif()

set(aziotsharedutil_target_libs )

if(${use_http})
    if(WIN32)
        if (WINCE)
            set(aziotsharedutil_target_libs ${aziotsharedutil_target_libs} crypt32 ws2)
        else()
            set(aziotsharedutil_target_libs ${aziotsharedutil_target_libs} crypt32 winhttp)
        endif()
    else()
        # try pkg-config first
        find_package(PkgConfig)
        if(PKG_CONFIG_FOUND)
            pkg_check_modules(CURL libcurl)
        endif()

        # if that didn't work, try CMake find_package
        if(NOT CURL_FOUND)
            find_package(CURL)
        endif()

        set(CURL_FIND_REQUIRED 1)
        find_package_handle_standard_args(CURL DEFAULT_MSG CURL_LIBRARIES)

        include_directories(${CURL_INCLUDE_DIRS})
        set(aziotsharedutil_target_libs ${aziotsharedutil_target_libs} ${CURL_LIBRARIES})
    endif()
endif()

if(${use_schannel})
    if(WIN32)
        if (WINCE)
            set(aziotsharedutil_target_libs ${aziotsharedutil_target_libs} secur32)
        else()
            set(aziotsharedutil_target_libs ${aziotsharedutil_target_libs} crypt32 ws2_32 secur32 advapi32)
        endif()
    endif()
endif()

if(${use_openssl})
    set(aziotsharedutil_target_libs ${aziotsharedutil_target_libs} ${OPENSSL_LIBRARIES})
    if (WIN32)
        set(aziotsharedutil_target_libs ${aziotsharedutil_target_libs} crypt32 ws2_32 secur32)
    endif()
endif()

if(${use_wolfssl})
    set(aziotsharedutil_target_libs ${aziotsharedutil_target_libs} wolfssl)
endif()

if(${use_cyclonessl} AND WIN32)
    set(aziotsharedutil_target_libs ${aziotsharedutil_target_libs} cyclonessl)
endif()

if(WIN32)
    if (NOT ${use_default_uuid})
        set(aziotsharedutil_target_libs ${aziotsharedutil_target_libs} rpcrt4.lib)
    endif()
endif()

if(LINUX)
    set(aziotsharedutil_target_libs ${aziotsharedutil_target_libs} pthread m rt)
    if (NOT ${use_default_uuid})
        set(aziotsharedutil_target_libs ${aziotsharedutil_target_libs} uuid)
    endif()
endif()

target_link_libraries(aziotsharedutil ${aziotsharedutil_target_libs})
target_link_libraries(aziotsharedutil_dll ${aziotsharedutil_target_libs})

if (${run_unittests} OR ${run_e2e_tests})
    include("dependencies-test.cmake")
    add_subdirectory(testtools)
    if (${run_unittests})
        add_subdirectory(tests)
    endif()
endif()

function(FindDllFromLib var libFile)
    get_filename_component(_libName ${libFile} NAME_WE)
    get_filename_component(_libDir ${libFile} DIRECTORY)

    while(NOT ("${_libDir}" STREQUAL ""))
        file(GLOB_RECURSE _dllList "${_libDir}/${_libName}.dll")
        list(LENGTH _dllList _listLen)
        if(${_listLen} GREATER 0)
            list(GET _dllList 0 _dll)
            set(${var} "${_dll}" PARENT_SCOPE)
            break()
        endif()
        get_filename_component(_libDir ${_libDir} DIRECTORY)
    endwhile()
endfunction()

if (NOT ${skip_samples})
    if(${use_openssl} AND WIN32)
        FindDllFromLib(SSL_DLL "${OPENSSL_SSL_LIBRARY}")
        FindDllFromLib(CRYPTO_DLL "${OPENSSL_CRYPTO_LIBRARY}")
    endif()
    add_subdirectory(samples)
endif()

# Set CMAKE_INSTALL_* if not defined
include(GNUInstallDirs)

if(${use_installed_dependencies})

    if(NOT DEFINED CMAKE_INSTALL_LIBDIR)
        set(CMAKE_INSTALL_LIBDIR "lib")
    endif()

    # Install Azure C Shared Utility
    set(package_location "cmake")

    install (TARGETS aziotsharedutil aziotsharedutil_dll EXPORT aziotsharedutilTargets
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_LIBDIR}/../bin
        INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/azureiot
    )
    install (FILES ${source_h_files} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/azureiot/azure_c_shared_utility)
    install (FILES ${micromock_h_files_full_path} ${INSTALL_H_FILES} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/azureiot)

    include(CMakePackageConfigHelpers)

    write_basic_package_version_file(
        "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}/${PROJECT_NAME}ConfigVersion.cmake"
        VERSION ${C_SHARED_VERSION}
        COMPATIBILITY SameMajorVersion
    )

    configure_file("configs/${PROJECT_NAME}Config.cmake"
        "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}/${PROJECT_NAME}Config.cmake"
        COPYONLY
    )

    install(EXPORT aziotsharedutilTargets
        FILE
            "${PROJECT_NAME}Targets.cmake"
        DESTINATION
            ${package_location}
    )
    install(
        FILES
            "configs/${PROJECT_NAME}Config.cmake"
            "configs/${PROJECT_NAME}Functions.cmake"
            "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}/${PROJECT_NAME}ConfigVersion.cmake"
        DESTINATION
            ${package_location}
    )
else()
	set(install_staticlibs
		aziotsharedutil
	)
	install(FILES ${source_h_files}
		DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/azure_c_shared_utility) 
	install(TARGETS ${install_staticlibs} 
		ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR})
endif()
