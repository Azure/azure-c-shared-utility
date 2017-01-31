#Copyright (c) Microsoft. All rights reserved.
#Licensed under the MIT license. See LICENSE file in the project root for full license information.

#this is CMakeLists.txt for httpapi_compact_ut
cmake_minimum_required(VERSION 2.8.11)

if(NOT ${use_http})
	message(FATAL_ERROR "httpapi_compact_ut being generated without HTTP support")
endif()

compileAsC11()
set(theseTestsName httpapi_compact_ut)

set(${theseTestsName}_test_files
httpapicompact_ut.c
)

set(${theseTestsName}_c_files
../../adapters/httpapi_compact.c
)

set(${theseTestsName}_h_files
)

build_c_test_artifacts(${theseTestsName} ON "tests/azure_c_shared_utility_tests")
