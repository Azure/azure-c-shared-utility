#Copyright (c) Microsoft. All rights reserved.
#Licensed under the MIT license. See LICENSE file in the project root for full license information.

#this is CMakeLists.txt for connectionstringparser_ut
cmake_minimum_required(VERSION 2.8.11)

compileAsC99()
set(theseTestsName connectionstringparser_ut)

set(${theseTestsName}_test_files
${theseTestsName}.c
)

include_directories(${SHARED_UTIL_REAL_TEST_FOLDER})

set(${theseTestsName}_c_files
    ../real_test_files/real_map.c
    ../real_test_files/real_string_tokenizer.c
    ../real_test_files/real_strings.c
    ${SHARED_UTIL_SRC_FOLDER}/crt_abstractions.c
    ${SHARED_UTIL_SRC_FOLDER}/connection_string_parser.c
)

set(${theseTestsName}_h_files
    ../real_test_files/real_map.h
    ../real_test_files/real_string_tokenizer.h
    ../real_test_files/real_strings.h
)

build_c_test_artifacts(${theseTestsName} ON "tests/azure_c_shared_utility_tests")
