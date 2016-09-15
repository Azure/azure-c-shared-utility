// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include <IPAddress.h>
#include "sslClient_arduino.h"
#include "azure_c_shared_utility/xlogging.h"

#ifdef ARDUINO_ARCH_ESP8266
#include "ESP8266WiFi.h"
#include "WiFiClientSecure.h"
#elif ARDUINO_SAMD_FEATHER_M0
#include "Adafruit_WINC1500.h"
#include "Adafruit_WINC1500Client.h"
#include "Adafruit_WINC1500SSLClient.h"
#else
#include "WiFi101.h"
#include "WiFiSSLClient.h"
#endif

SSLCLIENT_HANDLE sslClient_new(void)
{
	Client* sslClient;

#ifdef ARDUINO_ARCH_ESP8266
	sslClient = new WiFiClientSecure(); // for ESP8266
#elif ARDUINO_SAMD_FEATHER_M0
	sslClient = new Adafruit_WINC1500SSLClient(); // for Adafruit WINC1500
#else
	sslClient = new WiFiSSLClient();
#endif
	
	return (SSLCLIENT_HANDLE)sslClient;
}

void sslClient_delete(SSLCLIENT_HANDLE handle)
{
    if (handle == NULL)
    {
        LogError("NULL handle.");
    }
    else
    {
        Client* sslClient = (Client*)handle;
        delete sslClient;
    }
}

void sslClient_setTimeout(SSLCLIENT_HANDLE handle, unsigned long timeout)
{
    if (handle == NULL)
    {
        LogError("NULL handle.");
    }
    else
    {
        Client* sslClient = (Client*)handle;
        sslClient->setTimeout(timeout);
    }
}

uint8_t sslClient_connected(SSLCLIENT_HANDLE handle)
{
    uint8_t result;
    if (handle == NULL)
    {
        LogError("NULL handle.");
        result = (uint8_t)false;
    }
    else
    {
        Client* sslClient = (Client*)handle;
        result = (uint8_t)sslClient->connected();
    }
    return result;
}

int sslClient_connect(SSLCLIENT_HANDLE handle, uint32_t ipAddress, uint16_t port)
{
    int result;
    if (handle == NULL)
    {
        LogError("NULL handle.");
        result = 0;
    }
    else
    {
        Client* sslClient = (Client*)handle;
        IPAddress* ip = new IPAddress(ipAddress);
        result = (int)sslClient->connect(*ip, port);
    }
    return result;
}

void sslClient_stop(SSLCLIENT_HANDLE handle)
{
    if (handle == NULL)
    {
        LogError("NULL handle.");
    }
    else
    {
        Client* sslClient = (Client*)handle;
        sslClient->stop();
    }
}

size_t sslClient_write(SSLCLIENT_HANDLE handle, const uint8_t *buf, size_t size)
{
    size_t result;
    if (handle == NULL)
    {
        LogError("NULL handle.");
        result = (size_t)0;
    }
    else
    {
        Client* sslClient = (Client*)handle;
        result = sslClient->write(buf, size);
    }
    return result;
}

int sslClient_read(SSLCLIENT_HANDLE handle, uint8_t *buf, size_t size)
{
    int result;
    if (handle == NULL)
    {
        LogError("NULL handle.");
        result = 0;
    }
    else
    {
        Client* sslClient = (Client*)handle;
        result = sslClient->read(buf, size);
    }
    return result;
}

uint8_t sslClient_hostByName(const char* hostName, uint32_t* ipAddress)
{
    IPAddress ip;
    uint8_t result = WiFi.hostByName(hostName, ip);
    (*ipAddress) = (uint32_t)ip;
    return result;
}

