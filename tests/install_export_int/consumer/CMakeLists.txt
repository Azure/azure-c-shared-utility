#Copyright (c) Microsoft. All rights reserved.
#Licensed under the MIT license. See LICENSE file in the project root for full license information.

# Standalone consumer of the INSTALLED azure_c_shared_utility package.
#
# This project is deliberately not part of the azure_c_shared_utility build: it
# is configured separately by install_export_test.cmake against an install
# prefix, so it sees the package exactly the way a downstream project does.

cmake_minimum_required(VERSION 3.18)

# C and CXX are both enabled because the installed azure_iot_build_rules.cmake,
# which azure_c_shared_utilityConfig.cmake includes, probes CXX compiler flags.
project(azure_c_shared_utility_install_export_consumer C CXX)

find_package(azure_c_shared_utility CONFIG REQUIRED)

# CMake itself refuses to generate when an imported target names an include
# directory that does not exist, but it reports only the first one. Checking
# every entry of every imported target up front turns that into a single,
# explicit message naming the offending target and directory.
function(assert_exported_include_dirs_exist target)
    if(NOT TARGET ${target})
        message(FATAL_ERROR "the installed package did not define the imported target '${target}'")
    endif()

    get_target_property(exported_dirs ${target} INTERFACE_INCLUDE_DIRECTORIES)
    if(NOT exported_dirs)
        message(FATAL_ERROR "imported target '${target}' exports no INTERFACE_INCLUDE_DIRECTORIES")
    endif()

    foreach(exported_dir IN LISTS exported_dirs)
        if(NOT IS_DIRECTORY "${exported_dir}")
            message(FATAL_ERROR
                "imported target '${target}' exports the include directory '${exported_dir}', "
                "but the install does not create it. INCLUDES DESTINATION and the header "
                "install rules disagree.")
        endif()
    endforeach()
endfunction()

# An existing-but-wrong include root still fails the consumer at compile time,
# so assert that the headers are actually reachable through the exported roots
# with the spelling consumers use.
function(assert_header_reachable target header)
    get_target_property(exported_dirs ${target} INTERFACE_INCLUDE_DIRECTORIES)

    set(found FALSE)
    foreach(exported_dir IN LISTS exported_dirs)
        if(EXISTS "${exported_dir}/${header}")
            set(found TRUE)
            break()
        endif()
    endforeach()

    if(NOT found)
        message(FATAL_ERROR
            "'${header}' is not reachable through the include directories exported by "
            "'${target}': ${exported_dirs}")
    endif()
endfunction()

assert_exported_include_dirs_exist(aziotsharedutil)
assert_header_reachable(aziotsharedutil "azure_c_shared_utility/xlogging.h")
assert_header_reachable(aziotsharedutil "azure_c_shared_utility/strings.h")
assert_header_reachable(aziotsharedutil "azure_c_shared_utility/azure_base64.h")

# c_logging_v2 and c_logging_v2_core are re-exported through the same export set
# and so inherit the same INCLUDES DESTINATION, while their headers are installed
# under a different root. The public xlogging.h includes "c_logging/logger.h",
# so that root has to be exported too.
if(TARGET c_logging_v2)
    assert_exported_include_dirs_exist(c_logging_v2)
    assert_header_reachable(c_logging_v2 "c_logging/logger.h")
    assert_header_reachable(aziotsharedutil "c_logging/logger.h")
endif()

if(TARGET c_logging_v2_core)
    assert_exported_include_dirs_exist(c_logging_v2_core)
endif()

# macro_utils_generated.h, which the public headers pull in, defines
# MU_THE_NTH_ARG with 141 parameters. MSVC's traditional preprocessor caps macro
# parameters at 127 and rejects the header with C1112, so the conforming
# preprocessor has to be selected. It is available from VS2019 16.6, and
# macro_utils.h already refuses to build on anything older than VS2019.
function(use_conforming_preprocessor target)
    if(MSVC)
        target_compile_options(${target} PRIVATE /Zc:preprocessor)
    endif()
endfunction()

# The assertions above describe the contract; the build below proves it, by
# compiling and linking a translation unit that includes the public headers and
# calls into the library using nothing but the imported target.
add_executable(install_export_consumer main.c)
target_link_libraries(install_export_consumer aziotsharedutil)
use_conforming_preprocessor(install_export_consumer)

add_custom_command(TARGET install_export_consumer POST_BUILD
    COMMAND "$<TARGET_FILE:install_export_consumer>"
    WORKING_DIRECTORY "$<TARGET_FILE_DIR:install_export_consumer>"
    COMMENT "Running the consumer built against the installed package")

# When the project is built as dynamic there is a second exported library target,
# which carries the same exported include directories and must be equally usable.
# It is compiled and linked but not executed, so that the test does not depend on
# the platform's shared library search path.
if(TARGET aziotsharedutil_dll)
    assert_exported_include_dirs_exist(aziotsharedutil_dll)

    add_executable(install_export_consumer_dll main.c)
    target_link_libraries(install_export_consumer_dll aziotsharedutil_dll)
    use_conforming_preprocessor(install_export_consumer_dll)
endif()
