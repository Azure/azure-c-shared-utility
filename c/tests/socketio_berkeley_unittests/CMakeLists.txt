#Copyright (c) Microsoft. All rights reserved.
#Licensed under the MIT license. See LICENSE file in the project root for full license information.

cmake_minimum_required(VERSION 3.0)

if(MSVC)
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /IGNORE:4217")
set(CMAKE_SHARED_LINKER_FLAGS "$(CMAKE_SHARED_LINKER_FLAGS) /IGNORE:4217")
endif()

compileAsC99()
set(theseTestsName socketio_win32_unittests)
set(${theseTestsName}_cpp_files
${theseTestsName}.cpp
)

set(${theseTestsName}_c_files
../../adapter/socketio_berkeley.c
)

set(${theseTestsName}_h_files
)

build_test_artifacts(${theseTestsName} ON)