// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include "azure_c_shared_utility/platform.h"
#include "azure_c_shared_utility/xio.h"
#include "azure_c_shared_utility/tlsio_openssl.h"

#include <stdlib.h>
#include <unistd.h>
#include <sys/utsname.h>

#define TMP_FILE_NAME "/tmp/.msft_os_version_info_delete_me"
#define DESCRIPTION_COMMAND "/usr/bin/lsb_release -ds"
#define DEVICE_NULL "/dev/null"

static char* read_string_from_file(const char *fileName)
{
    char *retValue;
    FILE *fd = fopen(fileName, "rx");

    if (fd == NULL)
    {
        LogError("failed to open file '%s'", fileName);
        retValue = NULL;
    }
    else
    {
        if (fseek(fd, 0L, SEEK_END) != 0)
        {
            LogError("failed to find the end of file('%s')", fileName);
            retValue = NULL;
        }
        else
        {
            long size = ftell(fd);
            if ((size <= 0) || (fseek(fd, 0L, SEEK_SET) != 0))
            {
                LogError("failed to rewind the file");
                retValue = NULL;
            }
            else
            {
                retValue = malloc((size + 1) * sizeof(char));
                if (retValue == NULL)
                {
                    LogError("failed to allocate buffer for data");
                }
                else
                {
                    char *result = fgets(retValue, size, fd);
                    if (result == NULL)
                    {
                        LogError("fgets failed");
                        free(retValue);
                        retValue = NULL;
                    }
                    else
                    {
                        retValue = result;
                    }
                }
            }
        }
        fclose(fd);
    }
    return retValue;
}

static int local_system(const char *command, const char *outFile)
{
    char *outputFileName = (outFile == NULL) ? DEVICE_NULL : TMP_FILE_NAME;
    size_t  length = snprintf(NULL, 0, "%s > %s", command, outputFileName);
    char   *buffer = malloc(length + 1);
    int     retValue;
    if (buffer == NULL)
    {
        LogError("failed to allocate memory for command");
        retValue = -1;
    }
    else
    {
        (void)sprintf(buffer, "%s > %s", command, outputFileName);
        retValue = system(buffer);
        free(buffer);

        if (retValue != 0)
        {
            LogError("system '%s' failed", buffer);
        }
    }

    return retValue;
}

int platform_init(void)
{
    return tlsio_openssl_init();
}

const IO_INTERFACE_DESCRIPTION* platform_get_default_tlsio(void)
{
    return tlsio_openssl_get_interface_description();
}

STRING_HANDLE platform_get_platform_info(void)
{
    STRING_HANDLE result;

    // get description  (ubuntu, debian, etc...)
    if (local_system(DESCRIPTION_COMMAND, TMP_FILE_NAME) == 0)
    {
        char *description = read_string_from_file(TMP_FILE_NAME);
        if (unlink(TMP_FILE_NAME) == 0)
        {
            LogInfo("WARNING: failed to delete file '%s'", TMP_FILE_NAME);
        }

        struct utsname nnn;
        uname(&nnn);
        result = STRING_construct_sprintf("(%s; %s)", description, nnn.machine);
        free(description);
    }
    else
    {
        result = NULL;
    }

    return result;
}

void platform_deinit(void)
{
	tlsio_openssl_deinit();
}
