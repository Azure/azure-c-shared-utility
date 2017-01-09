#Copyright (c) Microsoft. All rights reserved.
#Licensed under the MIT license. See LICENSE file in the project root for full license information.

if(${use_installed_dependencies})
    find_package(libwebsockets REQUIRED CONFIG)
    include_directories(${LIBWEBSOCKETS_INCLUDE_DIRS})
else()
    include_directories(deps/libwebsockets/lib)
    include_directories(${PROJECT_BINARY_DIR}/deps/libwebsockets)
    if(${use_openssl})
        if(NOT OPENSSL_FOUND)
            message(FATAL_ERROR "dependencies.cmake was included before find_package(OpenSSL REQUIRED) was called")
        endif()
        # we already found OpenSSL libs/includes; feed them to libwebsockets
        set(LWS_OPENSSL_LIBRARIES ${OPENSSL_LIBRARIES} CACHE PATH "")
        set(LWS_OPENSSL_INCLUDE_DIRS ${OPENSSL_INCLUDE_DIR} CACHE PATH "")
    endif()
    add_subdirectory(deps/libwebsockets)
endif()