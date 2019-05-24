#Copyright (c) Microsoft. All rights reserved.
#Licensed under the MIT license. See LICENSE file in the project root for full license information.

if(NOT ${use_installed_dependencies})
    if (NOT TARGET azure_macro_utils_c)
        if (EXISTS ${CMAKE_CURRENT_LIST_DIR}/deps/azure-macro-utils-c/CMakeLists.txt)
            add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/deps/azure-macro-utils-c)
        else()
            message(FATAL_ERROR "Could not find azure-macro-utils-c source")
        endif()
    endif()
    if (NOT TARGET umock_c)
        if (EXISTS ${CMAKE_CURRENT_LIST_DIR}/deps/umock-c/CMakeLists.txt)
            add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/deps/umock-c)
	else()
	    message(FATAL_ERROR "Could not find umock-c source")
	endif()
    endif()
else()
    find_package(azure_macro_utils_c REQUIRED CONFIG)
    find_package(umock_c REQUIRED CONFIG)
endif()

include_directories(${MACRO_UTILS_INC_FOLDER})
include_directories(${UMOCK_C_INC_FOLDER})

if (${original_run_unittests} OR ${original_run_e2e_tests})
    include("${CMAKE_CURRENT_LIST_DIR}/dependencies-test.cmake")
    add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/testtools)

    setTargetBuildProperties(ctest)
    setTargetBuildProperties(testrunnerswitcher)
    setTargetBuildProperties(umock_c)
endif()
