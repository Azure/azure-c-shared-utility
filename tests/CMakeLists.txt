#Copyright (c) Microsoft. All rights reserved.
#Licensed under the MIT license. See LICENSE file in the project root for full license information.

#this is CMakeLists.txt for the folder tests of C shared utility
add_subdirectory(agenttime_ut)
add_subdirectory(base64_ut)
add_subdirectory(buffer_ut)
if(${use_condition})
    add_subdirectory(condition_ut)
endif()
add_subdirectory(constbuffer_ut)
add_subdirectory(constmap_ut)
add_subdirectory(crtabstractions_ut)
add_subdirectory(doublylinkedlist_ut)
add_subdirectory(gballoc_ut)
add_subdirectory(gballoc_without_init_ut)
add_subdirectory(hmacsha256_ut)
if(${use_http})
	add_subdirectory(httpapiex_ut)
	add_subdirectory(httpapiexsas_ut)
	add_subdirectory(httpheaders_ut)
    add_subdirectory(httpapicompact_ut)
endif()
add_subdirectory(singlylinkedlist_ut)
add_subdirectory(lock_ut)
add_subdirectory(map_ut)
add_subdirectory(refcount_ut)
add_subdirectory(sastoken_ut)
add_subdirectory(connectionstringparser_ut)
if(WIN32)
	add_subdirectory(socketio_win32_ut)
    add_subdirectory(x509_schannel_ut)
else()
	add_subdirectory(socketio_berkeley_ut)
endif()

#normally, with proper include paths, the below tests can be run under windows too.
#however, because of the setup involved, they are restricted to Linux
if(${use_openssl})
add_subdirectory(x509_openssl_ut)
endif()

add_subdirectory(string_tokenizer_ut)
add_subdirectory(strings_ut)
add_subdirectory(tickcounter_ut)
add_subdirectory(uniqueid_ut)
add_subdirectory(urlencode_ut)
add_subdirectory(vector_ut)
add_subdirectory(xio_ut)
add_subdirectory(optionhandler_ut)

if(use_wsio)
    add_subdirectory(uws_client_ut)
    add_subdirectory(uws_frame_encoder_ut)
endif()

#Add adapters tests
add_subdirectory(tlsioarduino_ut)
add_subdirectory(platformarduino_ut)
if(use_cyclonessl AND WIN32)
	add_subdirectory(tlsio_cyclonessl_ut)
	add_subdirectory(tlsio_cyclonessl_socket_bsd_ut)
endif()
add_subdirectory(utf8_checker_ut)

#Add template as reference for new tests
add_subdirectory(template_ut)
