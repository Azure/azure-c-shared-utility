#Copyright (c) Microsoft. All rights reserved.
#Licensed under the MIT license. See LICENSE file in the project root for full license information.

set(theseTestsName platform_linux_ut)

generate_cppunittest_wrapper(${theseTestsName})

set(${theseTestsName}_c_files
    platform_linux_mocked.c
)

set(${theseTestsName}_h_files
)

build_c_test_artifacts(${theseTestsName} ON "tests/azure_c_shared_utility_tests")
