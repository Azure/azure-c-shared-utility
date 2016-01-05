#Copyright (c) Microsoft. All rights reserved.
#Licensed under the MIT license. See LICENSE file in the project root for full license information.

#this is unittests for ctest
cmake_minimum_required(VERSION 2.8.11)

set(theseTestsName ctest_unittests)

set(${theseTestsName}_cpp_files
simpletestsuiteonetestcpp.cpp
)

set(${theseTestsName}_c_files
assertfailurestests.c
assertsuccesstests.c
ctestunittests.c
simpletestsuiteonetest.c
simpletestsuitetwotests.c
testfunctioncleanuptests.c
testfunctioninitializetests.c
testsuiteinitializecleanuptests.c
)

set(${theseTestsName}_h_files
)

build_test_artifacts(${theseTestsName} OFF)


