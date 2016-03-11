#Copyright (c) Microsoft. All rights reserved.
#Licensed under the MIT license. See LICENSE file in the project root for full license information.

#this is CMakeLists for umock_c
cmake_minimum_required(VERSION 2.8.11)

set(umock_c_c_files
./src/umock_c.c
./src/umockvalue.c
./src/umockcall.c
./src/umockvalue_charptr.c
./src/umockvalue_stdint.c
)

set(umock_c_h_files
./inc/umock_c.h
./inc/umock_c_internal.h
./inc/umockvalue.h
./inc/umockvalue_charptr.h
./inc/umockvalue_stdint.h
./inc/umockcall.h
)

IF(WIN32)
	#windows needs this define
	add_definitions(-D_CRT_SECURE_NO_WARNINGS)
ENDIF()

add_library(umock_c ${umock_c_c_files} ${umock_c_h_files})

#these are the include folders
#the following "set" statetement exports across the project a global variable called UMOCK_C_INC_FOLDER that expands to whatever needs to included when using umock_c library
set(UMOCK_C_INC_FOLDER ${CMAKE_CURRENT_LIST_DIR}/inc CACHE INTERNAL "this is what needs to be included if using umock_c lib" FORCE)

include_directories(${UMOCK_C_INC_FOLDER})

add_subdirectory(tests)
