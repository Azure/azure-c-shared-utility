#Copyright (c) Microsoft. All rights reserved.
#Licensed under the MIT license. See LICENSE file in the project root for full license information.

#this is CMakeLists.txt for $ws_url_ut
cmake_minimum_required(VERSION 2.8.11)

compileAsC11()
set(theseTestsName ws_url_ut)

set(${theseTestsName}_test_files
${theseTestsName}.c
)

set(${theseTestsName}_c_files
../../src/ws_url.c
../real_test_files/real_string_token.c
)

set(${theseTestsName}_h_files
)

build_c_test_artifacts(${theseTestsName} ON "tests/azure_c_shared_utility_tests")
