#Copyright (c) Microsoft. All rights reserved.
#Licensed under the MIT license. See LICENSE file in the project root for full license information.

#this is CMakeLists for micromock unittests
cmake_minimum_required(VERSION 2.8.11)

set(theseTestsName umock_c_without_init_unittests)

set(${theseTestsName}_c_files
umock_c_without_init_unittests.cpp
../../src/umock_c.c
../../src/umockcall.c
../../src/umockcallrecorder.c
../../src/umocktypes.c
../../src/umocktypes_charptr.c
../../src/umocktypes_c.c
../../src/umocktypename.c
)

umockc_build_test_artifacts(${theseTestsName} ON)
