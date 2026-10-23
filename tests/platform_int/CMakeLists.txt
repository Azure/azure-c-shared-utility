#Copyright (c) Microsoft. All rights reserved.
#Licensed under the MIT license. See LICENSE file in the project root for full license information.

# Plain executable, not a ctest suite: ctest logs through c-logging and needs logger_init, which this test
# must not call. A missing logger_init in platform_init makes the first log line abort the process.
add_executable(platform_int_exe platform_int.c)
target_link_libraries(platform_int_exe aziotsharedutil)
set_target_properties(platform_int_exe PROPERTIES FOLDER "tests/azure_c_shared_utility_tests")
add_test(NAME platform_int COMMAND platform_int_exe)
