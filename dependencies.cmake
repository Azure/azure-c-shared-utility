#Copyright (c) Microsoft. All rights reserved.
#Licensed under the MIT license. See LICENSE file in the project root for full license information.

if(${use_installed_dependencies})
        find_package(libwebsockets REQUIRED CONFIG)
        include_directories(${LIBWEBSOCKETS_INCLUDE_DIRS})
else()
        include_directories(deps/libwebsockets/lib)
        include_directories(${PROJECT_BINARY_DIR}/deps/libwebsockets)
        add_subdirectory(deps/libwebsockets)
endif()