#ifndef CL_HELPER
#define CL_HELPER

#include <CL/opencl.h>

bool cl_Err(cl_int err, const char * name);
cl_device_id cl_getDevice(cl_platform_id& platform);
void clInfo();

#endif