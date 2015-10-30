#Copyright (c) Microsoft. All rights reserved.
#Licensed under the MIT license. See LICENSE file in the project root for full license information.

cmake_minimum_required(VERSION 3.0)

compileAsC99()
#these are the C source files
set(source_c_files
./src/base64.c
./src/buffer.c
./src/crt_abstractions.c
./src/doublylinkedlist.c
./src/gballoc.c
./src/hmac.c
./src/hmacsha256.c
./src/io.c
./src/list.c
./src/map.c
./src/sastoken.c
./src/sha1.c
./src/sha224.c
./src/sha384-512.c
./src/strings.c
./src/string_tokenizer.c
./src/urlencode.c
./src/usha.c
./src/vector.c
./adapters/agenttime.c
)

if(${use_http})
	set(source_c_files ${source_c_files}
		./src/httpapiex.c
		./src/httpapiexsas.c
		./src/httpheaders.c
	)
endif()

#these are the C headers
set(source_h_files
./inc/agenttime.h
./inc/base64.h
./inc/buffer_.h
./inc/crt_abstractions.h
#./inc/condition.h
./inc/doublylinkedlist.h
./inc/gballoc.h
./inc/hmac.h
./inc/hmacsha256.h
./inc/io.h
./inc/iot_logging.h
./inc/list.h
./inc/lock.h
./inc/macro_utils.h
./inc/map.h
./inc/platform.h
./inc/sastoken.h
./inc/sha-private.h
./inc/sha.h
./inc/socketio.h
./inc/stdint_ce6.h
./inc/strings.h
./inc/string_tokenizer.h
./inc/threadapi.h
./inc/tlsio.h
./inc/urlencode.h
./inc/vector.h
)

if(${use_http})
	set(source_h_files ${source_h_files}
		./inc/httpapi.h
		./inc/httpapiex.h
		./inc/httpapiexsas.h
		./inc/httpheaders.h
	)
endif()

#these are the include folders
#the following "set" statetement exports across the project a global variable called COMMON_INC_FOLDER that expands to whatever needs to included when using COMMON library
set(SHARED_UTIL_INC_FOLDER ${CMAKE_CURRENT_LIST_DIR}/inc CACHE INTERNAL "this is what needs to be included if using sharedLib lib" FORCE)
set(SHARED_UTIL_SRC_FOLDER ${CMAKE_CURRENT_LIST_DIR}/src CACHE INTERNAL "this is what needs to be included when doing include sources" FORCE)
include_directories(${SHARED_UTIL_INC_FOLDER})

IF(WIN32)
	#windows needs this define
	add_definitions(-D_CRT_SECURE_NO_WARNINGS)
ENDIF(WIN32)

#Setup the platform files
set(source_h_files ${source_h_files}
	${PLATFORM_C_FILE}
	${TLSIO_C_FILE}
	${SOCKETIO_C_FILE}
	${LOCK_C_FILE}
	${THREAD_C_FILE}
	${HTTP_C_FILE}
)

#this is the product (a library)
add_library(aziotsharedlib ${source_c_files} ${source_h_files})

add_subdirectory(tests)
add_subdirectory(testtools)