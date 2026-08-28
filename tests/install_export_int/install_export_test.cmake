#Copyright (c) Microsoft. All rights reserved.
#Licensed under the MIT license. See LICENSE file in the project root for full license information.

# End to end check of the CMake install/export contract. Run with cmake -P.
#
# The project is configured, built and installed into a scratch prefix using the
# PACKAGING configuration (run_unittests=OFF), and a standalone consumer is then
# configured and built against that prefix with
# find_package(azure_c_shared_utility CONFIG REQUIRED).
#
# Installing from the enclosing test build would not do: the header install rule
# that populates ${CMAKE_INSTALL_INCLUDEDIR}/azureiot only has files to install
# when the test tooling is built, so a test-enabled install creates directories
# that a real packaging install does not, and hides exactly the defect this test
# exists to catch.

cmake_minimum_required(VERSION 3.18)

foreach(required_variable SOURCE_DIR CONSUMER_DIR WORK_DIR)
    if(NOT DEFINED ${required_variable})
        message(FATAL_ERROR "${required_variable} must be defined on the command line")
    endif()
endforeach()

set(install_prefix "${WORK_DIR}/prefix")
set(library_build_dir "${WORK_DIR}/library-build")
set(consumer_build_dir "${WORK_DIR}/consumer-build")

# Always start from nothing: a stale prefix from an earlier run could supply a
# directory that the current install rules do not create.
file(REMOVE_RECURSE "${WORK_DIR}")
file(MAKE_DIRECTORY "${library_build_dir}" "${consumer_build_dir}")

function(run_step description)
    execute_process(
        COMMAND ${ARGN}
        RESULT_VARIABLE step_result
        OUTPUT_VARIABLE step_stdout
        ERROR_VARIABLE step_stderr)

    if(NOT step_result EQUAL 0)
        string(REPLACE ";" " " printable_command "${ARGN}")
        message(FATAL_ERROR
            "${description} failed (exit code ${step_result})\n"
            "command: ${printable_command}\n"
            "--- stdout ---\n${step_stdout}\n"
            "--- stderr ---\n${step_stderr}")
    endif()

    message(STATUS "${description}: OK")
endfunction()

# ---------------------------------------------------------------------------
# Configure, build and install the project the way a package would.
# ---------------------------------------------------------------------------

set(configure_command
    "${CMAKE_COMMAND}"
    -S "${SOURCE_DIR}"
    -B "${library_build_dir}"
    "-DCMAKE_INSTALL_PREFIX=${install_prefix}"
    -Drun_unittests:BOOL=OFF
    -Drun_int_tests:BOOL=OFF
    -Drun_e2e_tests:BOOL=OFF
    -Drun_valgrind:BOOL=OFF
    -Dskip_samples:BOOL=ON)

if(GENERATOR)
    list(APPEND configure_command -G "${GENERATOR}")
endif()
if(GENERATOR_PLATFORM)
    list(APPEND configure_command -A "${GENERATOR_PLATFORM}")
endif()
if(GENERATOR_TOOLSET)
    list(APPEND configure_command -T "${GENERATOR_TOOLSET}")
endif()
if(C_COMPILER)
    list(APPEND configure_command "-DCMAKE_C_COMPILER=${C_COMPILER}")
endif()
if(CXX_COMPILER)
    list(APPEND configure_command "-DCMAKE_CXX_COMPILER=${CXX_COMPILER}")
endif()
if(BUILD_TYPE)
    list(APPEND configure_command "-DCMAKE_BUILD_TYPE=${BUILD_TYPE}")
endif()

# FORWARDED_OPTIONS carries the enclosing build's feature switches, so that the
# scratch build produces the same set of exported targets as the build under test.
foreach(forwarded_option IN LISTS FORWARDED_OPTIONS)
    list(APPEND configure_command "-D${forwarded_option}")
endforeach()

run_step("configuring the project for install" ${configure_command})

set(build_command "${CMAKE_COMMAND}" --build "${library_build_dir}")
if(BUILD_TYPE)
    list(APPEND build_command --config "${BUILD_TYPE}")
endif()
if(PARALLEL_LEVEL)
    list(APPEND build_command --parallel ${PARALLEL_LEVEL})
endif()

run_step("building the project" ${build_command})

set(install_command "${CMAKE_COMMAND}" --install "${library_build_dir}")
if(BUILD_TYPE)
    list(APPEND install_command --config "${BUILD_TYPE}")
endif()

run_step("installing the project" ${install_command})

# ---------------------------------------------------------------------------
# Consume the install prefix the way a downstream project does.
# ---------------------------------------------------------------------------

set(consumer_configure_command
    "${CMAKE_COMMAND}"
    -S "${CONSUMER_DIR}"
    -B "${consumer_build_dir}"
    "-DCMAKE_PREFIX_PATH=${install_prefix}")

if(GENERATOR)
    list(APPEND consumer_configure_command -G "${GENERATOR}")
endif()
if(GENERATOR_PLATFORM)
    list(APPEND consumer_configure_command -A "${GENERATOR_PLATFORM}")
endif()
if(GENERATOR_TOOLSET)
    list(APPEND consumer_configure_command -T "${GENERATOR_TOOLSET}")
endif()
if(C_COMPILER)
    list(APPEND consumer_configure_command "-DCMAKE_C_COMPILER=${C_COMPILER}")
endif()
if(CXX_COMPILER)
    list(APPEND consumer_configure_command "-DCMAKE_CXX_COMPILER=${CXX_COMPILER}")
endif()
if(BUILD_TYPE)
    list(APPEND consumer_configure_command "-DCMAKE_BUILD_TYPE=${BUILD_TYPE}")
endif()

# This is the step that reproduces the reported failure: CMake refuses to
# generate when an imported target names an include directory that the install
# did not create.
run_step("configuring a consumer against the installed package" ${consumer_configure_command})

set(consumer_build_command "${CMAKE_COMMAND}" --build "${consumer_build_dir}")
if(BUILD_TYPE)
    list(APPEND consumer_build_command --config "${BUILD_TYPE}")
endif()

# Building the consumer compiles the public headers through the exported include
# directories and, via a POST_BUILD command, runs the resulting executable.
run_step("building and running the consumer" ${consumer_build_command})

message(STATUS "install/export contract verified against ${install_prefix}")
