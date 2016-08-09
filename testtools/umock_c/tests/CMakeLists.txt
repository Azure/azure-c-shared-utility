#Copyright (c) Microsoft. All rights reserved.
#Licensed under the MIT license. See LICENSE file in the project root for full license information.

#this is CMakeLists for micromock unittests folder, it is empty
cmake_minimum_required(VERSION 2.8.11)

add_subdirectory(umockalloc_ut)
add_subdirectory(umockcall_ut)
add_subdirectory(umockcallpairs_ut)
add_subdirectory(umockcallrecorder_ut)
add_subdirectory(umock_c_ut)
add_subdirectory(umock_c_wout_init_ut)
add_subdirectory(umocktypename_ut)
add_subdirectory(umocktypes_ut)
add_subdirectory(umocktypes_wout_init_ut)
add_subdirectory(umocktypes_bool_ut)
add_subdirectory(umocktypes_c_ut)
add_subdirectory(umocktypes_charptr_ut)
add_subdirectory(umocktypes_stdint_ut)
add_subdirectory(umock_c_negt_noini_ut)
add_subdirectory(umock_c_negt_ut)
add_subdirectory(umock_c_int)
add_subdirectory(umock_c_negt_int)
add_subdirectory(umock_c_gen_func_decl_int)
add_subdirectory(umock_c_malloc_hook_int)
add_subdirectory(umock_c_ptrarg_leak_int)
