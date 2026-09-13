// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.


#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

// This file is OS-specific, and is identified by setting include directories
// in the project
#include "socket_async_os.h"

#include "azure_c_shared_utility/dns_resolver.h"
#include "azure_c_shared_utility/gballoc.h"
#include "azure_c_shared_utility/crt_abstractions.h"
#include "azure_c_shared_utility/xlogging.h"
#include "ares.h"

// EXTRACT_IPV4 pulls the uint32_t IPv4 address out of an addrinfo struct
// The default definition handles lwIP. Please add comments for other systems tested.
#define EXTRACT_IPV4(ptr) ((struct sockaddr_in *) ptr->ai_addr)->sin_addr.s_addr

#ifdef IPV6_ENABLED
// EXTRACT_IPV6 pulls the uint32_t IPv6 address out of an addrinfo struct
#define EXTRACT_IPV6(ptr) ((struct sockaddr_in6 *) ptr->ai_addr)->sin6_addr.s6_addr
#endif // IPV6_ENABLED

typedef struct
{
    char* hostname;
    int port;
    uint32_t ip_v4;
#ifdef IPV6_ENABLED
    uint8_t ip_v6[16];
#endif // IPV6_ENABLED
    bool is_complete;
    bool is_failed;
    bool in_progress;
    struct addrinfo* addrInfo;
    ares_channel ares_resolver;
} DNSRESOLVER_INSTANCE;

DNSRESOLVER_HANDLE dns_resolver_create(const char* hostname, int port, const DNSRESOLVER_OPTIONS* options)
{
    /* Codes_SRS_dns_resolver_30_012: [ The optional options parameter shall be ignored. ]*/
    DNSRESOLVER_INSTANCE* result;
    int status;
    (void)options;
    if (hostname == NULL)
    {
        /* Codes_SRS_dns_resolver_30_011: [ If the hostname parameter is NULL, dns_resolver_create shall log an error and return NULL. ]*/
        LogError("NULL hostname");
        result = NULL;
    }
    else
    {
        result = calloc(1, sizeof(DNSRESOLVER_INSTANCE));
        if (result == NULL)
        {
            /* Codes_SRS_dns_resolver_30_014: [ On any failure, dns_resolver_create shall log an error and return NULL. ]*/
            LogError("malloc instance failed");
            result = NULL;
        }
        else
        {
            int ms_result;
            result->is_complete = false;
            result->is_failed = false;
            result->in_progress = false;
            result->ip_v4 = 0;
#ifdef IPV6_ENABLED
            memset(result->ip_v6, 0, sizeof(result->ip_v6)); // zero out the IPv6 address
#endif // IPV6_ENABLED
            result->port = port;
            /* Codes_SRS_dns_resolver_30_010: [ dns_resolver_create shall make a copy of the hostname parameter to allow immediate deletion by the caller. ]*/
            ms_result = mallocAndStrcpy_s(&result->hostname, hostname);
            if (ms_result != 0)
            {
                /* Codes_SRS_dns_resolver_30_014: [ On any failure, dns_resolver_create shall log an error and return NULL. ]*/
                LogError("dns_resolver_create: hostname allocation failed");
                free(result);
                result = NULL;
            }
            else
            {
                status = ares_library_init(ARES_LIB_INIT_ALL);
                if (status != ARES_SUCCESS)
                {
                    LogError("ares_library_init failed: %s", ares_strerror(status));
                    free(result->hostname);
                    free(result);
                    result = NULL;
                }
                else 
                {
                    status = ares_init(&(result->ares_resolver));
                    if(status != ARES_SUCCESS)
                    {
                        LogError("ares_init failed: %s", ares_strerror(status));
                        ares_library_cleanup();
                        free(result->hostname);
                        free(result);
                        result = NULL;
                    }
                }
            }
        }
    }
    
    if(result != NULL)
    {
        result->addrInfo = NULL;
    }

    return result;
}

// Every exit path out of the query callback must end the lookup, otherwise
// dns_resolver_is_lookup_complete never returns true and the caller polls forever.
static void complete_lookup(DNSRESOLVER_INSTANCE* dns, bool failed)
{
    dns->is_failed = failed;
    dns->is_complete = true;
    dns->in_progress = false;
}

static void release_addrinfo(DNSRESOLVER_INSTANCE* dns)
{
    if (dns->addrInfo != NULL)
    {
        if (dns->addrInfo->ai_addr != NULL)
        {
            free(dns->addrInfo->ai_addr);
        }
        free(dns->addrInfo);
        dns->addrInfo = NULL;
    }
}

static void query_completed_cb(void *arg, int status, int timeouts, struct hostent *he)
{
    struct addrinfo *ptr = NULL;
    struct sockaddr_in *addr;
#ifdef IPV6_ENABLED
    struct sockaddr_in6 *addr6;
    const uint8_t zero_ip_v6[16] = { 0 };
#endif // IPV6_ENABLED

    DNSRESOLVER_INSTANCE *dns = (DNSRESOLVER_INSTANCE *)arg;
    (void)timeouts;

    /* Codes_SRS_dns_resolver_30_022: [ If the DNS lookup process has completed, dns_resolver_is_create_complete shall return true. ]*/
    if (status != ARES_SUCCESS)
    {
        LogError("Failed DNS lookup for %s: %d (%s)", dns->hostname, status, ares_strerror(status));
        complete_lookup(dns, true);
    }
    else if (he == NULL || he->h_addr_list == NULL || he->h_addr_list[0] == NULL)
    {
        LogError("Failed DNS lookup for %s: no address returned", dns->hostname);
        complete_lookup(dns, true);
    }
#ifdef IPV6_ENABLED
    else if (he->h_addrtype == AF_INET6)
    {
        if (he->h_length != (int)sizeof(struct in6_addr))
        {
            LogError("Failed DNS lookup for %s: unexpected IPv6 address length %d", dns->hostname, he->h_length);
            complete_lookup(dns, true);
        }
        else if ((dns->addrInfo = calloc(1, sizeof(struct addrinfo))) == NULL)
        {
            LogError("dns addrInfo: allocation failed");
            complete_lookup(dns, true);
        }
        else if ((dns->addrInfo->ai_addr = calloc(1, sizeof(struct sockaddr_in6))) == NULL)
        {
            LogError("dns addrinfo ai_addr: allocation failed");
            release_addrinfo(dns);
            complete_lookup(dns, true);
        }
        else
        {
            ptr = dns->addrInfo;
            addr6 = (void *)ptr->ai_addr;

            memcpy(&addr6->sin6_addr, he->h_addr_list[0], sizeof(struct in6_addr));
            addr6->sin6_family = AF_INET6;
            addr6->sin6_port = htons((unsigned short)dns->port);

            memcpy(dns->ip_v6, EXTRACT_IPV6(ptr), 16); // IPv6 address is 16 bytes
            dns->addrInfo->ai_addrlen = sizeof(struct sockaddr_in6);
            dns->addrInfo->ai_family = AF_INET6;
            dns->addrInfo->ai_socktype = SOCK_STREAM;
            dns->addrInfo->ai_protocol = IPPROTO_TCP;

            /* Codes_SRS_dns_resolver_30_033: [ If dns_resolver_is_create_complete has returned true and the lookup process has failed, dns_resolver_get_ipv4 shall return 0. ]*/
            complete_lookup(dns, memcmp(dns->ip_v6, zero_ip_v6, sizeof(zero_ip_v6)) == 0);
        }
    }
#endif // IPV6_ENABLED
    else if (he->h_addrtype == AF_INET)
    {
        if (he->h_length != (int)sizeof(struct in_addr))
        {
            LogError("Failed DNS lookup for %s: unexpected IPv4 address length %d", dns->hostname, he->h_length);
            complete_lookup(dns, true);
        }
        else if ((dns->addrInfo = calloc(1, sizeof(struct addrinfo))) == NULL)
        {
            LogError("dns addrInfo: allocation failed");
            complete_lookup(dns, true);
        }
        else if ((dns->addrInfo->ai_addr = calloc(1, sizeof(struct sockaddr_in))) == NULL)
        {
            LogError("dns addrinfo ai_addr: allocation failed");
            release_addrinfo(dns);
            complete_lookup(dns, true);
        }
        else
        {
            ptr = dns->addrInfo;
            addr = (void *)ptr->ai_addr;

            memcpy(&addr->sin_addr, he->h_addr_list[0], sizeof(struct in_addr));
            addr->sin_family = he->h_addrtype;
            addr->sin_port = htons((unsigned short)dns->port);

            dns->ip_v4 = EXTRACT_IPV4(ptr);
            dns->addrInfo->ai_addrlen = sizeof(struct sockaddr_in);
            dns->addrInfo->ai_family = AF_INET;
            dns->addrInfo->ai_socktype = SOCK_STREAM;
            dns->addrInfo->ai_protocol = IPPROTO_TCP;

            /* Codes_SRS_dns_resolver_30_033: [ If dns_resolver_is_create_complete has returned true and the lookup process has failed, dns_resolver_get_ipv4 shall return 0. ]*/
            complete_lookup(dns, dns->ip_v4 == 0);
        }
    }
    else
    {
        LogError("Failed DNS lookup for %s: unexpected address family %d", dns->hostname, he->h_addrtype);
        complete_lookup(dns, true);
    }
}

// ares_getsock reports which of the channel's sockets are ready, and only writes
// the entries it reports. A zero bitmask leaves socks untouched, so no socket may
// be derived from it; ares_process_fd is still called with ARES_SOCKET_BAD so that
// c-ares can time the outstanding query out.
static void process_ares_sockets(DNSRESOLVER_INSTANCE* dns)
{
    ares_socket_t socks[ARES_GETSOCK_MAXNUM];
    int bitmask;
    int i;
    bool any_socket_ready = false;

    bitmask = ares_getsock(dns->ares_resolver, socks, ARES_GETSOCK_MAXNUM);

    for (i = 0; i < ARES_GETSOCK_MAXNUM; i++)
    {
        bool readable = ARES_GETSOCK_READABLE(bitmask, i) != 0;
        bool writable = ARES_GETSOCK_WRITABLE(bitmask, i) != 0;

        if (readable || writable)
        {
            any_socket_ready = true;
            ares_process_fd(dns->ares_resolver,
                readable ? socks[i] : ARES_SOCKET_BAD,
                writable ? socks[i] : ARES_SOCKET_BAD);
        }
    }

    if (!any_socket_ready)
    {
        ares_process_fd(dns->ares_resolver, ARES_SOCKET_BAD, ARES_SOCKET_BAD);
    }
}

/* Codes_SRS_dns_resolver_30_021: [ dns_resolver_is_create_complete shall perform the asynchronous work of DNS lookup and log any errors. ]*/
bool dns_resolver_is_lookup_complete(DNSRESOLVER_HANDLE dns_in)
{
    DNSRESOLVER_INSTANCE* dns = (DNSRESOLVER_INSTANCE*)dns_in;

    bool result;
    if (dns == NULL)
    {
        /* Codes_SRS_dns_resolver_30_020: [ If the dns parameter is NULL, dns_resolver_is_create_complete shall log an error and return false. ]*/
        LogError("NULL dns");
        result = false;
    }
    else if (dns->is_complete)
    {
        /* Codes_SRS_dns_resolver_30_024: [ If dns_resolver_is_create_complete has previously returned true, dns_resolver_is_create_complete shall do nothing and return true. ]*/
        result = true;
    }
    else
    {
        if (!dns->in_progress)
        {
            // Set before the call because c-ares may invoke the callback synchronously,
            // for instance when the name is served from the hosts file.
            dns->in_progress = true;
#ifdef IPV6_ENABLED
            ares_gethostbyname(dns->ares_resolver, dns->hostname, AF_UNSPEC, query_completed_cb, (void*)dns);
#else
            ares_gethostbyname(dns->ares_resolver, dns->hostname, AF_INET, query_completed_cb, (void*)dns);
#endif // IPV6_ENABLED
        }
        else
        {
            process_ares_sockets(dns);
        }

        /* Codes_SRS_dns_resolver_30_023: [ If the DNS lookup process is not yet complete, dns_resolver_is_create_complete shall return false. ]*/
        /* Codes_SRS_dns_resolver_30_022: [ If the DNS lookup process has completed, dns_resolver_is_create_complete shall return true. ]*/
        result = dns->is_complete;
    }

    return result;
}


void dns_resolver_destroy(DNSRESOLVER_HANDLE dns_in)
{
    DNSRESOLVER_INSTANCE* dns = (DNSRESOLVER_INSTANCE*)dns_in;
    if (dns == NULL)
    {
        /* Codes_SRS_dns_resolver_30_050: [ If the dns parameter is NULL, dns_resolver_destroy shall log an error and do nothing. ]*/
        LogError("NULL dns");
    }
    else
    {
        /* Codes_SRS_dns_resolver_30_051: [ dns_resolver_destroy shall delete all acquired resources and delete the DNSRESOLVER_HANDLE. ]*/
        ares_destroy(dns->ares_resolver);
        ares_library_cleanup();

        release_addrinfo(dns);
        free(dns->hostname);
        free(dns);
    }
}

uint32_t dns_resolver_get_ipv4(DNSRESOLVER_HANDLE dns_in)
{
    DNSRESOLVER_INSTANCE* dns = (DNSRESOLVER_INSTANCE*)dns_in;
    uint32_t result;
    if (dns == NULL)
    {
        /* Codes_SRS_dns_resolver_30_030: [ If the dns parameter is NULL, dns_resolver_get_ipv4 shall log an error and return 0. ]*/
        LogError("NULL dns");
        result = 0;
    }
    else
    {
        if (dns->is_complete)
        {
            if (dns->is_failed)
            {
                /* Codes_SRS_dns_resolver_30_033: [ If dns_resolver_is_create_complete has returned true and the lookup process has failed, dns_resolver_get_ipv4 shall return 0. ]*/
                result = 0;
            }
            else
            {
                /* Codes_SRS_dns_resolver_30_032: [ If dns_resolver_is_create_complete has returned true and the lookup process has succeeded, dns_resolver_get_ipv4 shall return the discovered IPv4 address. ]*/
                result = dns->ip_v4;
            }
        }
        else
        {
            /* Codes_SRS_dns_resolver_30_031: [ If dns_resolver_is_create_complete has not yet returned true, dns_resolver_get_ipv4 shall log an error and return 0. ]*/
            LogError("dns_resolver_get_ipv4 when not complete");
            result = 0;
        }
    }
    return result;
}

struct addrinfo* dns_resolver_get_addrInfo(DNSRESOLVER_HANDLE dns_in)
{
    DNSRESOLVER_INSTANCE* dns = (DNSRESOLVER_INSTANCE*)dns_in;
    struct addrinfo* result;
    if (dns == NULL)
    {
        /* Codes_SRS_dns_resolver_30_030: [ If the dns parameter is NULL, dns_resolver_get_ipv4 shall log an error and return 0. ]*/
        LogError("NULL dns");
        result = NULL;
    }
    else
    {
        if (dns->is_complete)
        {
            if (dns->is_failed)
            {
                /* Codes_SRS_dns_resolver_30_033: [ If dns_resolver_is_create_complete has returned true and the lookup process has failed, dns_resolver_get_ipv4 shall return 0. ]*/
                result = NULL;
            }
            else
            {
                /* Codes_SRS_dns_resolver_30_032: [ If dns_resolver_is_create_complete has returned true and the lookup process has succeeded, dns_resolver_get_ipv4 shall return the discovered IPv4 address. ]*/
                result = dns->addrInfo;
            }
        }
        else
        {
            /* Codes_SRS_dns_resolver_30_031: [ If dns_resolver_is_create_complete has not yet returned true, dns_resolver_get_ipv4 shall log an error and return 0. ]*/
            LogError("dns_resolver_get_ipv4 when not complete");
            result = NULL;
        }
    }
    return result;
}
