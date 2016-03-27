#Copyright (c) Microsoft. All rights reserved.
#Licensed under the MIT license. See LICENSE file in the project root for full license information.

#this is unittests for ctest
cmake_minimum_required(VERSION 2.8.11)

set(ctest_unittests_c_files
assertfailurestests.c
assertsuccesstests.c
ctestunittests.c
simpletestsuiteonetest.c
simpletestsuitetwotests.c
testfunctioncleanuptests.c
testfunctioninitializetests.c
testsuiteinitializecleanuptests.c
)

set(whentestsuiteinitializefailstests_c_files
	whentestsuiteinitializefailstests.c
	whentestsuiteinitializefailstests_main.c
)

set(whentestfunctioninitializefailstests_c_files
	whentestfunctioninitializefailstests.c
	whentestfunctioninitializefailstests_main.c
)

set(ctest_unittests_cpp_files
simpletestsuiteonetestcpp.cpp
)

set(ctest_unittests_h_files
)

include_directories(${CTEST_INC_FOLDER})

IF(WIN32)
	#windows needs this define
	add_definitions(-D_CRT_SECURE_NO_WARNINGS)
ENDIF(WIN32)

add_executable(
ctest_unittests ${ctest_unittests_c_files} ${ctest_unittests_cpp_files} ${ctest_unittests_h_files}
)

add_executable(
	whentestsuiteinitializefailstests ${whentestsuiteinitializefailstests_c_files} ${ctest_unittests_h_files}
)

add_executable(
	whentestfunctioninitializefailstests ${whentestfunctioninitializefailstests_c_files} ${ctest_unittests_h_files}
)

set_target_properties(ctest
			   PROPERTIES
			   FOLDER "SharedTestTools")
			   
target_link_libraries(ctest_unittests ctest)
target_link_libraries(whentestsuiteinitializefailstests ctest)
target_link_libraries(whentestfunctioninitializefailstests ctest)

if(NOT ${skip_unittests})
	add_test(NAME ctest_unittests COMMAND ctest_unittests)
	add_test(NAME whentestsuiteinitializefailstests COMMAND whentestsuiteinitializefailstests)
	add_test(NAME whentestfunctioninitializefailstests COMMAND whentestfunctioninitializefailstests)
endif()