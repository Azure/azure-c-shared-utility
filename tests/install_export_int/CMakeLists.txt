#Copyright (c) Microsoft. All rights reserved.
#Licensed under the MIT license. See LICENSE file in the project root for full license information.

# Regression test for the CMake install/export rules.
#
# The exported include directories of the installed targets have to agree with
# where the headers are actually installed. When they do not, every downstream
# project that does find_package(azure_c_shared_utility CONFIG) fails, either at
# generate time with "Imported target ... includes non-existent path" or at
# compile time on a header that cannot be found. Neither failure is visible from
# inside this project's own build, so it is checked here end to end.

set(theseTestsName install_export_int)

# Feature switches that change which targets are exported, or what the exported
# targets need from the include path, are forwarded to the scratch build.
set(forwarded_options
    "use_openssl:BOOL=${use_openssl}"
    "use_wolfssl:BOOL=${use_wolfssl}"
    "use_mbedtls:BOOL=${use_mbedtls}"
    "use_bearssl:BOOL=${use_bearssl}"
    "use_http:BOOL=${use_http}"
    "use_wsio:BOOL=${use_wsio}"
    "use_condition:BOOL=${use_condition}"
    "use_default_uuid:BOOL=${use_default_uuid}"
    "use_installed_dependencies:BOOL=${use_installed_dependencies}"
    "build_as_dynamic:BOOL=${build_as_dynamic}")

include(ProcessorCount)
ProcessorCount(host_processor_count)
if(host_processor_count EQUAL 0)
    set(host_processor_count 1)
endif()

get_property(generator_is_multi_config GLOBAL PROPERTY GENERATOR_IS_MULTI_CONFIG)

# Single config generators take the configuration at configure time, multi config
# generators take it per build, and the test has to drive the scratch build the
# same way ctest was invoked.
if(generator_is_multi_config)
    set(configuration_arguments "-DCONFIG=$<CONFIG>")
else()
    set(configuration_arguments "-DBUILD_TYPE=${CMAKE_BUILD_TYPE}")
endif()

if(CMAKE_CROSSCOMPILING)
    # The test builds a consumer and runs it, which a cross compiling build
    # cannot do. The install/export rules are host independent, so this costs no
    # coverage as long as at least one native leg runs the test.
    message(STATUS "${theseTestsName}: not registered, the consumer cannot be run when cross compiling")
else()
    add_test(NAME ${theseTestsName}
        COMMAND ${CMAKE_COMMAND}
            "-DSOURCE_DIR=${CMAKE_CURRENT_LIST_DIR}/../.."
            "-DCONSUMER_DIR=${CMAKE_CURRENT_LIST_DIR}/consumer"
            "-DWORK_DIR=${CMAKE_CURRENT_BINARY_DIR}/scratch"
            "-DGENERATOR=${CMAKE_GENERATOR}"
            "-DGENERATOR_PLATFORM=${CMAKE_GENERATOR_PLATFORM}"
            "-DGENERATOR_TOOLSET=${CMAKE_GENERATOR_TOOLSET}"
            "-DC_COMPILER=${CMAKE_C_COMPILER}"
            "-DCXX_COMPILER=${CMAKE_CXX_COMPILER}"
            "-DTOOLCHAIN_FILE=${CMAKE_TOOLCHAIN_FILE}"
            ${configuration_arguments}
            "-DPARALLEL_LEVEL=${host_processor_count}"
            "-DFORWARDED_OPTIONS=${forwarded_options}"
            -P "${CMAKE_CURRENT_LIST_DIR}/install_export_test.cmake")

    # The test configures and builds the project from scratch, so it needs a much
    # larger budget than a unit test.
    set_tests_properties(${theseTestsName} PROPERTIES
        TIMEOUT 3600
        LABELS "integration;packaging")
endif()
