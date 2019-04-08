// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#ifndef GBNETWORK_H
#define GBNETWORK_H

#include "azure_c_shared_utility/umock_c_prod.h"

#ifdef __cplusplus
#include <cstddef>
#include <cstdlib>
#include <cstdint>
extern "C"
{
#else
#include <stddef.h>
#include <stdlib.h>
#include <stdint.h>
#endif

#ifdef WIN32
    #include <winsock2.h>
#else
    #include <sys/types.h>
    #include <sys/socket.h>
    #include <sys/select.h>
    #include <netdb.h>
#endif

/* all translation units that need network measurement need to have GB_MEASURE_NETWORK_FOR_THIS defined */
/* GB_DEBUG_NETWORK is the switch that turns the measurement on/off, so that it is not on always */
#if defined(GB_DEBUG_NETWORK)

MOCKABLE_FUNCTION(, int, gbnetwork_init);
MOCKABLE_FUNCTION(, void, gbnetwork_deinit);

MOCKABLE_FUNCTION(, int, gbnetwork_socket, int, socket_family, int, socket_type, int, protocol);
MOCKABLE_FUNCTION(, int, gbnetwork_getaddrinfo, const char*, node, const char*, service, const struct addrinfo*, hints, struct addrinfo**, res);
MOCKABLE_FUNCTION(, void, gbnetwork_freeaddrinfo, struct addrinfo*, res);
MOCKABLE_FUNCTION(, int, gbnetwork_connect, int, sockfd, const struct sockaddr*, addr, socklen_t, addrlen);
MOCKABLE_FUNCTION(, int, gbnetwork_shutdown, int, socket, int, how)
MOCKABLE_FUNCTION(, int, gbnetwork_close, int, socket)
MOCKABLE_FUNCTION(, int, gbnetwork_getsockopt, int, fd, int, level, int, optname, void* __restrict, optval, socklen_t*__restrict, optlen);
MOCKABLE_FUNCTION(, int, gbnetwork_select, int, nfds, fd_set*__restrict, readfds, fd_set*__restrict, writefds, fd_set*__restrict, exceptfds, struct timeval*__restrict, timeout);

int gbnetwork_fcntl(int fildes, int cmd, ...);

#ifdef WIN32
MOCKABLE_FUNCTION(, int, gbnetwork_send, SOCKET, sock, const char*, buf, int, len, int, flags);
#else
MOCKABLE_FUNCTION(, ssize_t, gbnetwork_send, int, sock, const void*, buf, size_t, len, int, flags);
#endif

#ifdef WIN32
MOCKABLE_FUNCTION(, int, gbnetwork_recv, SOCKET, sock, char*, buf, int, len, int, flags);
#else
MOCKABLE_FUNCTION(, ssize_t, gbnetwork_recv, int, sock, void*, buf, size_t, len, int, flags);
#endif

MOCKABLE_FUNCTION(, uint64_t, gbnetwork_getBytesSent);
MOCKABLE_FUNCTION(, uint64_t, gbnetwork_getNumSends);
MOCKABLE_FUNCTION(, uint64_t, gbnetwork_getBytesRecv);
MOCKABLE_FUNCTION(, uint64_t, gbnetwork_getNumRecv);
MOCKABLE_FUNCTION(, void, gbnetwork_resetMetrics);

/* if GB_MEASURE_NETWORK_FOR_THIS is defined then we want to redirect network send functions to gbnetwork_xxx functions */
#ifdef GB_MEASURE_NETWORK_FOR_THIS
#define connect gbnetwork_connect
#define getaddrinfo gbnetwork_getaddrinfo
#define freeaddrinfo gbnetwork_freeaddrinfo
#define socket gbnetwork_socket
#define fcntl gbnetwork_fcntl
#define getsockopt gbnetwork_getsockopt
#define select gbnetwork_select
#define send gbnetwork_send
#define recv gbnetwork_recv
#endif

#else /* GB_DEBUG_NETWORK */

#define gbnetwork_init() 0
#define gbnetwork_deinit() ((void)0)
#define gbnetwork_getBytesSent() 0
#define gbnetwork_getNumSends() 0

#define gbnetwork_getBytesRecv() 0
#define gbnetwork_getNumRecv() 0

#endif /* GB_DEBUG_NETWORK */

#ifdef __cplusplus
}
#endif

#endif /* GBNETWORK_H */
