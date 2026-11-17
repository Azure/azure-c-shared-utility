#Copyright (c) Microsoft. All rights reserved.
#Licensed under the MIT license. See LICENSE file in the project root for full license information.

# c-build-tools' set_default_build_options() selects -std:c++23preview for MSVC 19.43+ with Visual Studio generators.
# CMake maps that switch to the C++-only LanguageStandard property starting with 3.31.7; older versions pass it
# through AdditionalOptions to C sources too, which fails mixed C/C++ targets with D8016 (/std:c17 vs /std:c++23preview).
# For those CMake versions, fall back to CMake's own C++23 option (-std:c++latest), which maps to LanguageStandard.
# Include after c-build-tools is added and before any dependency that calls set_default_build_options().

get_property(cxx23_visual_studio_generator_fix_applied GLOBAL PROPERTY CXX23_VISUAL_STUDIO_GENERATOR_FIX_APPLIED)
if ((NOT cxx23_visual_studio_generator_fix_applied) AND (COMMAND set_default_build_options) AND (CMAKE_GENERATOR MATCHES "^Visual Studio") AND (CMAKE_VERSION VERSION_LESS 3.31.7))
    set_property(GLOBAL PROPERTY CXX23_VISUAL_STUDIO_GENERATOR_FIX_APPLIED TRUE)

    # Redefining a command keeps the previous definition reachable with a leading underscore.
    macro(set_default_build_options)
        _set_default_build_options()
        if (CMAKE_CXX23_STANDARD_COMPILE_OPTION STREQUAL "-std:c++23preview")
            set(CMAKE_CXX23_STANDARD_COMPILE_OPTION "-std:c++latest")
            set(CMAKE_CXX23_EXTENSION_COMPILE_OPTION "-std:c++latest")
        endif()
    endmacro()
endif()
