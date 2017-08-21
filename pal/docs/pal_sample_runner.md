# pal_sample_runner

The pal_sample_runner component encapsulates a `main()` plus any device-dependent initialization and de-initialization that is necessary to run any of the samples contained in the Azure IoT C SDK. This component allows a wide range of Azure IoT sample programs with no duplicated init and de-init code.

Examples of initialization that the pal_sample_runner may need to perform include pin assignment, WiFi connection, NTP service connection, and starting a thread to run the sample.

The pal_sample_runner is not needed for production code; it is just for running samples.

### References
[pal_sample_runner.h](https://github.com/Azure/azure-c-shared-utility/blob/pal/pal/include/pal_sample_runner.h) **TODO: link->master** <br/>

## API

### azure_iot_sample_program

```
typedef void(*azure_iot_sample_program_t)();

azure_iot_sample_program_t azure_iot_sample_program;
```
Each Azure IoT C sample program will set its `run()` function into the global variable `azure_iot_sample_program`.

If the global variable `azure_iot_sample_program` is non-NULL, the `pal_sample_runner` module must:
* set up any device-level configuration that is needed, 
* run the provided sample program 
* perform any necessary device-level de-initialization,
* exit


