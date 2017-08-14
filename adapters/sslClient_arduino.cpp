// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include <IPAddress.h>
#include "sslClient_arduino.h"
#include "azure_c_shared_utility/xlogging.h"

#ifdef ARDUINO_ARCH_ESP8266
#include "ESP8266WiFi.h"
#include "WiFiClientSecure.h"
static WiFiClientSecure sslClient; // for ESP8266
#elif ARDUINO_ARCH_ESP32
#include "WiFi.h"
#include "WiFiClientSecure.h"
static WiFiClientSecure sslClient; // for ESP32
#else
#include "WiFi101.h"
#include "WiFiSSLClient.h"
static WiFiSSLClient sslClient;
#endif

void sslClient_setTimeout(unsigned long timeout)
{
    sslClient.setTimeout(timeout);
}

uint8_t sslClient_connected(void)
{
    return (uint8_t)sslClient.connected();
}

int sslClient_connect(uint32_t ipAddress, uint16_t port)
{
    IPAddress ip = IPAddress(ipAddress);
    return (int)sslClient.connect(ip, port);
}

void sslClient_stop(void)
{
    sslClient.stop();
}

size_t sslClient_write(const uint8_t *buf, size_t size)
{
    return sslClient.write(buf, size);
}

size_t sslClient_print(const char* str)
{
    return sslClient.print(str);
}

int sslClient_read(uint8_t *buf, size_t size)
{
    return sslClient.read(buf, size);
}

int sslClient_available(void)
{
    return sslClient.available();
}

uint8_t sslClient_hostByName(const char* hostName, uint32_t* ipAddress)
{
    IPAddress ip;
    uint8_t result = WiFi.hostByName(hostName, ip);
    (*ipAddress) = (uint32_t)ip;
    return result;
}

