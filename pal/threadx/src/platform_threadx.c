// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

// Copyright (c) Express Logic.  All rights reserved.
// Please contact support@expresslogic.com for any questions or use the support portal at www.rtos.com

/* This file is used for porting platform between threadx and azure-iot-sdk-c.  */

#include "tx_api.h"
#include "nx_api.h"
#include "nxd_dns.h"
#include "nx_secure_tls_api.h"
#include "tlsio_threadx.h"
#include "azure_c_shared_utility/platform.h"
#include "azure_c_shared_utility/xio.h"
#include "azure_c_shared_utility/xlogging.h"

extern void threadx_azure_sdk_initialize(void);
extern void threadx_azure_sdk_deinitialize(void);
extern void (*platform_driver_get())(NX_IP_DRIVER *);

/* Define the default thread priority, stack size, etc. The user can override this 
   via -D command line option or via project settings.  */

#ifndef THREADX_IP_STACK_SIZE
#define THREADX_IP_STACK_SIZE           (2048)
#endif /* THREADX_IP_STACK_SIZE  */

#ifndef THREADX_PACKET_COUNT
#define THREADX_PACKET_COUNT            (32)
#endif /* THREADX_PACKET_COUNT  */

#ifndef THREADX_PACKET_SIZE
#define THREADX_PACKET_SIZE             (576)
#endif /* THREADX_PACKET_SIZE  */

#define THREADX_POOL_SIZE               ((THREADX_PACKET_SIZE + sizeof(NX_PACKET)) * THREADX_PACKET_COUNT)

#ifndef THREADX_ARP_CACHE_SIZE
#define THREADX_ARP_CACHE_SIZE          512
#endif /* THREADX_ARP_CACHE_SIZE  */


/* Define the stack/cache for ThreadX.  */ 
static UCHAR threadx_ip_stack[THREADX_IP_STACK_SIZE];
static UCHAR threadx_pool_stack[THREADX_POOL_SIZE];
static UCHAR threadx_arp_cache_area[THREADX_ARP_CACHE_SIZE];


/* Define the prototypes for ThreadX.  */
NX_PACKET_POOL                          pool_0;
NX_IP                                   ip_0;
NX_DNS                                  dns_client;
NX_DNS                                  *_threadx_dns_client_created_ptr;


#ifndef THREADX_DHCP_DISABLE

#include "nxd_dhcp_client.h"
static NX_DHCP                          dhcp_client;
static void                             wait_dhcp(void);

#define THREADX_IPV4_ADDRESS            IP_ADDRESS(0, 0, 0, 0)
#define THREADX_IPV4_MASK               IP_ADDRESS(0, 0, 0, 0)

#else

#ifndef THREADX_IPV4_ADDRESS
//#define THREADX_IPV4_ADDRESS          IP_ADDRESS(192, 168, 100, 33)
#error "SYMBOL THREADX_IPV4_ADDRESS must be defined. This symbol specifies the IP address of device. "

#endif /* THREADX_IPV4_ADDRESS */
#ifndef THREADX_IPV4_MASK
//#define THREADX_IPV4_MASK             0xFFFFFF00UL
#error "SYMBOL THREADX_IPV4_MASK must be defined. This symbol specifies the IP address mask of device. "
#endif /* IPV4_MASK */

#ifndef THREADX_GATEWAY_ADDRESS
//#define THREADX_GATEWAY_ADDRESS       IP_ADDRESS(192, 168, 100, 1)
#error "SYMBOL THREADX_GATEWAY_ADDRESS must be defined. This symbol specifies the gateway address for routing. "
#endif /* THREADX_GATEWAY_ADDRESS */

#ifndef THREADX_DNS_SERVER_ADDRESS
//#define THREADX_DNS_SERVER_ADDRESS      IP_ADDRESS(192, 168, 100, 1)
#error "SYMBOL THREADX_DNS_SERVER_ADDRESS must be defined. This symbol specifies the dns server address for routing. "
#endif /* THREADX_DNS_SERVER_ADDRESS */

#endif /* THREADX_DHCP_DISABLE  */

static UINT dns_create();


int platform_init(void)
{

UINT    status;
ULONG   ip_address;
ULONG   network_mask;
ULONG   gateway_address;


    /* Initialize the NetX system.  */
    nx_system_initialize();

    /* Create a packet pool.  */
    status = nx_packet_pool_create(&pool_0, "NetX Main Packet Pool", THREADX_PACKET_SIZE,
                                   threadx_pool_stack , THREADX_POOL_SIZE);
    
    /* Check for pool creation error.  */
    if (status)
    {
        LogError("THREADX platform initialize fail: PACKET POOL CREATE FAIL.");
        return(status);
    }

    /* Create an IP instance for the DHCP Client. The rest of the DHCP Client set up is handled
       by the client thread entry function.  */
     status = nx_ip_create(&ip_0, "NetX IP Instance 0", THREADX_IPV4_ADDRESS, THREADX_IPV4_MASK,
                           &pool_0, platform_driver_get(), (UCHAR*)threadx_ip_stack, THREADX_IP_STACK_SIZE, 1);

    /* Check for IP create errors.  */
    if (status)
    {
        LogError("THREADX platform initialize fail: IP CREATE FAIL.");
        return(status);
    }

    /* Enable ARP and supply ARP cache memory for IP Instance 0.  */
    status = nx_arp_enable(&ip_0, (VOID *)threadx_arp_cache_area, THREADX_ARP_CACHE_SIZE);

    /* Check for ARP enable errors.  */
    if (status)
    {
        LogError("THREADX platform initialize fail: ARP ENABLE FAIL.");
        return(status);
    }

    /* Enable ICMP traffic.  */
    status = nx_icmp_enable(&ip_0);

    /* Check for ICMP enable errors.  */
    if (status)
    {
        LogError("THREADX platform initialize fail: ICMP ENABLE FAIL.");
        return(status);
    }

    /* Enable TCP traffic.  */
    status = nx_tcp_enable(&ip_0);

    /* Check for TCP enable errors.  */
    if (status)
    {
        LogError("THREADX platform initialize fail: TCP ENABLE FAIL.");
        return(status);
    }

    /* Enable UDP traffic.  */
    status = nx_udp_enable(&ip_0);

    /* Check for UDP enable errors.  */
    if (status)
    {
        LogError("THREADX platform initialize fail: UDP ENABLE FAIL.");
        return(status);
    }

#ifndef THREADX_DHCP_DISABLE
    wait_dhcp();
#else
    nx_ip_gateway_address_set(&ip_0, THREADX_GATEWAY_ADDRESS);
#endif /* THREADX_DHCP_DISABLE  */

    /* Get IP address and gateway address. */
    nx_ip_address_get(&ip_0, &ip_address, &network_mask);
    nx_ip_gateway_address_get(&ip_0, &gateway_address);

    /* Output IP address and gateway address. */
    LogInfo("IP address: %d.%d.%d.%d",
           (ip_address >> 24),
           (ip_address >> 16 & 0xFF),
           (ip_address >> 8 & 0xFF),
           (ip_address & 0xFF));
    LogInfo("Mask: %d.%d.%d.%d",
           (network_mask >> 24),
           (network_mask >> 16 & 0xFF),
           (network_mask >> 8 & 0xFF),
           (network_mask & 0xFF));
    LogInfo("Gateway: %d.%d.%d.%d",
           (gateway_address >> 24),
           (gateway_address >> 16 & 0xFF),
           (gateway_address >> 8 & 0xFF),
           (gateway_address & 0xFF));

    /* Ceate dns.  */
    status = dns_create();

    /* Check for DNS create errors.  */
    if (status)
    {
        LogError("THREADX platform initialize fail: DNS CREATE FAIL.");
        return(status);
    }

    /* Initialize TLS.  */
    nx_secure_tls_initialize();

    /* Initialize ThreadX Azure SDK.  */
    threadx_azure_sdk_initialize();

    return 0;
}

const IO_INTERFACE_DESCRIPTION* platform_get_default_tlsio(void)
{
    return (tlsio_threadx_get_interface_description());
}

STRING_HANDLE platform_get_platform_info(PLATFORM_INFO_OPTION options)
{
  
    // No applicable options, so ignoring parameter
    (void)options;

    // Expected format: "(<runtime name>; <operating system name>; <platform>)"

    return STRING_construct("(native; ThreadX; undefined)");
}

void platform_deinit(void)
{

    /* Cleanup the resource.  */
    threadx_azure_sdk_deinitialize();
}

#ifndef THREADX_DHCP_DISABLE
static void wait_dhcp(void)
{

ULONG   actual_status;

    LogInfo("DHCP In Progress...");

    /* Create the DHCP instance.  */
    nx_dhcp_create(&dhcp_client, &ip_0, "dhcp_client");

    /* Start the DHCP Client.  */
    nx_dhcp_start(&dhcp_client);

    /* Wait util address is solved. */
    nx_ip_status_check(&ip_0, NX_IP_ADDRESS_RESOLVED, &actual_status, NX_WAIT_FOREVER);
}
#endif /* THREADX_DHCP_DISABLE  */


static UINT dns_create()
{
      
UINT    status;
ULONG   dns_server_address[3];
UINT    dns_server_address_size = 12;
 
    /* Create a DNS instance for the Client.  Note this function will create
       the DNS Client packet pool for creating DNS message packets intended
       for querying its DNS server. */
    status = nx_dns_create(&dns_client, &ip_0, (UCHAR *)"DNS Client");
    if (status)
    {
        return(status);
    }

    /* Is the DNS client configured for the host application to create the pecket pool? */
#ifdef NX_DNS_CLIENT_USER_CREATE_PACKET_POOL   

    /* Yes, use the packet pool created above which has appropriate payload size
       for DNS messages. */
    status = nx_dns_packet_pool_set(&dns_client, ip_0.nx_ip_default_packet_pool);
    if (status)
    {
        nx_dns_delete(&dns_client);
        return(status);
    }
#endif /* NX_DNS_CLIENT_USER_CREATE_PACKET_POOL */  

#ifndef THREADX_DHCP_DISABLE
    /* Retrieve DNS server address.  */
    nx_dhcp_interface_user_option_retrieve(&dhcp_client, 0, NX_DHCP_OPTION_DNS_SVR, (UCHAR *)(dns_server_address), &dns_server_address_size); 
#else
    dns_server_address[0] = THREADX_DNS_SERVER_ADDRESS;
#endif    
    
    /* Add an IPv4 server address to the Client list. */
    status = nx_dns_server_add(&dns_client, dns_server_address[0]);
    if (status)
    {
        nx_dns_delete(&dns_client);
        return(status);
    }
    
    /* Record the dns client, it will be used in socketio_threadx.c  */
    _threadx_dns_client_created_ptr = &dns_client;
    
    /* Output DNS Server address.  */
    LogInfo("DNS Server address: %d.%d.%d.%d\r\n",
           (dns_server_address[0] >> 24),
           (dns_server_address[0] >> 16 & 0xFF),
           (dns_server_address[0] >> 8 & 0xFF),
           (dns_server_address[0] & 0xFF));
    
    return(0);
}
