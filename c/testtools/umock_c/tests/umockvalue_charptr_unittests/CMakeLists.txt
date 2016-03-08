#Copyright (c) Microsoft. All rights reserved.
#Licensed under the MIT license. See LICENSE file in the project root for full license information.

#this is CMakeLists for micromock unittests
cmake_minimum_required(VERSION 2.8.11)

set(theseTestsName umockvalue_charptr_unittests)

set(${theseTestsName}_c_files
umockvalue_charptr_unittests.cpp
)

set(${theseTestsName}_h_files
)

build_test_artifacts(${theseTestsName} ON)
