#Copyright (c) Microsoft. All rights reserved.
#Licensed under the MIT license. See LICENSE file in the project root for full license information.

cmake_minimum_required(VERSION 2.8.11)

compileAsC11()
set(theseTestsName xio_state_ut)

set(${theseTestsName}_test_files
${theseTestsName}.c
)

set(${theseTestsName}_c_files
${SHARED_UTIL_SRC_FOLDER}/vector.c
${SHARED_UTIL_SRC_FOLDER}/singlylinkedlist.c
${SHARED_UTIL_SRC_FOLDER}/crt_abstractions.c
${SHARED_UTIL_SRC_FOLDER}/xio_state.c
)

set(${theseTestsName}_h_files
${SHARED_UTIL_INC_FOLDER}/azure_c_shared_utility/xio_state.h
${SHARED_UTIL_INC_FOLDER}/azure_c_shared_utility/xio_adapter.h
gballoc_ut_impl_1.h
gballoc_ut_impl_2.h
callbacks.h
fake_adapter.h
message_processing.h
)

include_directories(.)

build_c_test_artifacts(${theseTestsName} ON "tests/azure_c_shared_utility_tests")
