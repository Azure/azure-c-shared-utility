#Copyright (c) Microsoft. All rights reserved.
#Licensed under the MIT license. See LICENSE file in the project root for full license information.

cmake_minimum_required(VERSION 2.8.11)

compileAsC11()
set(theseTestsName tlsio_openssl_compact_ut)

set(${theseTestsName}_test_files
${theseTestsName}.c
)

set(${theseTestsName}_c_files
../../adapters/tlsio_openssl_compact.c
../../src/singlylinkedlist.c
../../src/crt_abstractions.c
)

set(${theseTestsName}_h_files
ssl_impl.h
test_defines.h
gballoc_ut_impl_1.h
gballoc_ut_impl_2.h
callbacks.h
)

include_directories(.)

build_c_test_artifacts(${theseTestsName} ON "tests/azure_c_shared_utility_tests")
