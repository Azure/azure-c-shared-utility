#Copyright (c) Microsoft. All rights reserved.
#Licensed under the MIT license. See LICENSE file in the project root for full license information.

#this is CMakeLists.txt for condition_unittests
cmake_minimum_required(VERSION 2.8.11)

compileAsC11()
set(theseTestsName condition_unittests)

set(${theseTestsName}_cpp_files
	${theseTestsName}.cpp
)

set(${theseTestsName}_c_files
	${CONDITION_C_FILE}
	${LOCK_C_FILE}	
    ${THREAD_C_FILE}
    ../../adapters/agenttime.c
)

set(${theseTestsName}_h_files
)

build_test_artifacts(${theseTestsName} ON)
