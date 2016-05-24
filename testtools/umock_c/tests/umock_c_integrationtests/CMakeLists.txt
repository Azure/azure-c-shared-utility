#Copyright (c) Microsoft. All rights reserved.
#Licensed under the MIT license. See LICENSE file in the project root for full license information.

cmake_minimum_required(VERSION 2.8.11)

set(theseTestsName umock_c_integrationtests)

set(${theseTestsName}_test_files
umock_c_integrationtests.c
umock_c_generate_function_declaration_integrationtests.c
umock_c_ptrarg_leak_integrationtests.c
umock_c_malloc_hook_integrationtests.c
)

set(${theseTestsName}_c_files
../../src/umock_c.c
../../src/umockalloc.c
../../src/umockcall.c
../../src/umockcallrecorder.c
../../src/umocktypes.c
../../src/umocktypes_charptr.c
../../src/umocktypes_c.c
../../src/umocktypename.c
)

set(${theseTestsName}_h_files
test_dependency.h
)

umockc_build_test_artifacts(${theseTestsName} ON)
