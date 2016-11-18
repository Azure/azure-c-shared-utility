#Copyright (c) Microsoft. All rights reserved.
#Licensed under the MIT license. See LICENSE file in the project root for full license information.

#this is CMakeLists.txt for <myTestName>_ut
cmake_minimum_required(VERSION 2.8.11)

compileAsC11()
set(theseTestsName template_ut)

set(${theseTestsName}_test_files
${theseTestsName}.c
)

set(${theseTestsName}_c_files
##
# Include all target files that you need to execute the test.
# ../../adapters/<myTestOnAdapters>.c
# ../../src/<myTestOnSRC>.c
##
target/target.c
)

set(${theseTestsName}_h_files
##
# Include all headers that you need to execute the test. Normally we don't need any.
##
)

build_c_test_artifacts(${theseTestsName} ON "tests/azure_c_shared_utility_tests")
