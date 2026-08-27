#Copyright (c) Microsoft. All rights reserved.
#Licensed under the MIT license. See LICENSE file in the project root for full license information.

set(theseTestsName socketio_int)

generate_cppunittest_wrapper(${theseTestsName})

set(${theseTestsName}_c_files
)

set(${theseTestsName}_h_files
)

build_c_test_artifacts(${theseTestsName} ON "tests/azure_c_shared_utility_tests" ADDITIONAL_LIBS aziotsharedutil)
