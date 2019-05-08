#Copyright (c) Microsoft. All rights reserved.
#Licensed under the MIT license. See LICENSE file in the project root for full license information.

set(original_run_e2e_tests ${run_e2e_tests})
set(original_run_unittests ${run_unittests})
set(original_run_int_tests ${run_int_tests})

set(run_e2e_tests OFF)
set(run_unittests OFF)
set(run_int_tests OFF)

if(NOT ${use_installed_dependencies})
    if ((NOT TARGET azure_macro_utils_c) AND (EXISTS ${CMAKE_CURRENT_LIST_DIR}/deps/azure-macro-utils-c/CMakeLists.txt))
        add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/deps/azure-macro-utils-c)
    endif()
    if ((NOT TARGET umock-c) AND (EXISTS ${CMAKE_CURRENT_LIST_DIR}/deps/umock-c/CMakeLists.txt))
        add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/deps/umock-c)
    endif()
endif()

if(${use_installed_dependencies})
        find_package(umock_c REQUIRED CONFIG)
endif()

if (${original_run_unittests})
    include("dependencies-test.cmake")
    add_subdirectory(testtools)
endif()

if (${run_unittests} OR ${run_e2e_tests})
    setTargetBuildProperties(ctest)
    setTargetBuildProperties(testrunnerswitcher)
    setTargetBuildProperties(umock_c)
endif()

set(run_e2e_tests ${original_run_e2e_tests})
set(run_unittests ${original_run_unittests})
set(run_int_tests ${original_run_int_tests})

