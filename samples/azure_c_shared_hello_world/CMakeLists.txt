#Copyright (c) Microsoft. All rights reserved.
#Licensed under the MIT license. See LICENSE file in the project root for full license information.

compileAsC99()

set(azure_c_shared_hello_world_c_files
    main.c
)

IF(WIN32)
    #windows needs this define
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
    add_definitions(-DGB_MEASURE_MEMORY_FOR_THIS -DGB_DEBUG_ALLOC)
ENDIF(WIN32)

add_executable(azure_c_shared_hello_world ${azure_c_shared_hello_world_c_files})

target_link_libraries(azure_c_shared_hello_world 
    aziotsharedutil
)

set_target_properties(azure_c_shared_hello_world
			   PROPERTIES
			   FOLDER "azure_c_shared_utility_samples")
