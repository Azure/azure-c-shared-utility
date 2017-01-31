#Copyright (c) Microsoft. All rights reserved.
#Licensed under the MIT license. See LICENSE file in the project root for full license information.

#this is CMakeLists.txt for crt_abstractions_ut
cmake_minimum_required(VERSION 2.8.11)

compileAsC11()

set(theseTestsName crt_abstractions_ut)

set(${theseTestsName}_test_files
crtabstractions_ut.c
)

set(${theseTestsName}_c_files
../../src/crt_abstractions.c
)

set(${theseTestsName}_h_files
)

build_c_test_artifacts(${theseTestsName} ON "tests/azure_c_shared_utility_tests")