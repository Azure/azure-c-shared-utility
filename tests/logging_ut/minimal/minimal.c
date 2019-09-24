#include "azure_c_shared_utility/xlogging.h"

int main(void)
{

    LogError("some int has value %d", 42);

    LogInfo("some other int has value %d", 0x42);

#ifdef WIN32
    LogLastError("nothing is expected, but here's a parameter of type int = %d", '3');
#endif

    return 0;
}
