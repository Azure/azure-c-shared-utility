#Copyright (c) Microsoft. All rights reserved.
#Licensed under the MIT license. See LICENSE file in the project root for full license information.

compileAsC99()

set(iot_c_utility_c_files
    iot_c_utility.c
)

if (WIN32)
    #windows needs this define
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
    add_definitions(-DGB_MEASURE_MEMORY_FOR_THIS -DGB_DEBUG_ALLOC)
ENDIF(WIN32)

message(STATUS "is http on ${use_http}")
if (${use_http})
    add_definitions(-DUSE_HTTP)
endif()

add_executable(iot_c_utility ${iot_c_utility_c_files})

target_link_libraries(iot_c_utility 
    aziotsharedutil
)