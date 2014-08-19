#include "cl_helper.hpp"

#include <iostream>
#include <sstream>
#include <vector>

bool cl_Err(cl_int err, const char * name)
{
	if (err != CL_SUCCESS) {
		std::cerr << "ERROR: " << name
		<< " (" << err << ")" << std::endl;
		system ("PAUSE");
		exit(EXIT_FAILURE);
		return false;
	}
	return true;
}


std::string getDeviceInfo(cl_device_id device_id) {
	cl_int err;

	char deviceName[64];
	cl_long amountOfMemory;
	err = clGetDeviceInfo(device_id, CL_DEVICE_NAME, sizeof(deviceName), deviceName, NULL);
	cl_Err(err, "Device Name");
	err = clGetDeviceInfo(device_id, CL_DEVICE_GLOBAL_MEM_SIZE, sizeof(amountOfMemory), &amountOfMemory, NULL);
	cl_Err(err, "Device Memory");

	std::stringstream ss;

	ss << "OpenCL Device:" << deviceName << " | Memory:" << amountOfMemory/1048576 << "MB";

	return ss.str();
}

cl_device_id cl_getDevice(cl_platform_id& platform) {
	cl_int err;

	cl_device_id device_id;

	cl_uint numPlatforms;
    err = clGetPlatformIDs(0, NULL, &numPlatforms);
	cl_Err(err, "Platform Count");

	cl_platform_id * platformIDs = new cl_platform_id[numPlatforms];
	err = clGetPlatformIDs(numPlatforms, platformIDs, NULL);
	cl_Err(err, "Platform IDs");

	//Loop through all Platforms and get devices
	for(size_t i = 0; i < numPlatforms; i++) {
		cl_uint numDevices;
		err = clGetDeviceIDs(platformIDs[i], CL_DEVICE_TYPE_ALL,0, NULL, &numDevices);
		cl_Err(err, "Device Number");

		cl_device_id * deviceIDs = new cl_device_id[numDevices];
		err = clGetDeviceIDs(platformIDs[i], CL_DEVICE_TYPE_ALL,numDevices, deviceIDs, NULL);

		cl_Err(err, "Device Id's");

		//Print info about every device and use the last found GPU
		for(size_t j = 0; j < numDevices; j++) {
			char deviceName[64];
			cl_long amountOfMemory;
			err = clGetDeviceInfo(deviceIDs[j], CL_DEVICE_NAME, sizeof(deviceName), deviceName, NULL);
			cl_Err(err, "Device Name");
			err = clGetDeviceInfo(deviceIDs[j], CL_DEVICE_GLOBAL_MEM_SIZE, sizeof(amountOfMemory), &amountOfMemory, NULL);
			cl_Err(err, "Device Memory");

			cl_device_type deviceType;
            err = clGetDeviceInfo(deviceIDs[j],CL_DEVICE_TYPE,sizeof(cl_device_type), &deviceType, NULL);
			cl_Err(err, "Device Type");

			size_t max_work_sizes;
            err = clGetDeviceInfo(deviceIDs[j],CL_DEVICE_MAX_WORK_GROUP_SIZE,sizeof(size_t), &max_work_sizes, NULL);
			cl_Err(err, "Device Work Sizes");

			if(deviceType == CL_DEVICE_TYPE_GPU) {
				device_id = deviceIDs[j];
				platform = platformIDs[i];
			}

			std::cout << "OpenCL Device Found:" << deviceName << " | Memory:" << amountOfMemory/1048576 << "MB" << std::endl;
		}
	}

	std::cout << "Using " << getDeviceInfo(device_id) << std::endl;

	return device_id;
}


void clInfo() {
    char* value;
    size_t valueSize;
    cl_uint platformCount;
    cl_platform_id* platforms;
    cl_uint deviceCount;
    cl_device_id* devices;
    cl_uint maxComputeUnits;

    // get all platforms
    clGetPlatformIDs(0, NULL, &platformCount);
    platforms = (cl_platform_id*) malloc(sizeof(cl_platform_id) * platformCount);
    clGetPlatformIDs(platformCount, platforms, NULL);

    for (size_t i = 0; i < platformCount; i++) {

        // get all devices
        clGetDeviceIDs(platforms[i], CL_DEVICE_TYPE_ALL, 0, NULL, &deviceCount);
        devices = (cl_device_id*) malloc(sizeof(cl_device_id) * deviceCount);
        clGetDeviceIDs(platforms[i], CL_DEVICE_TYPE_ALL, deviceCount, devices, NULL);

        // for each device print critical attributes
        for (size_t j = 0; j < deviceCount; j++) {

            // print device name
            clGetDeviceInfo(devices[j], CL_DEVICE_NAME, 0, NULL, &valueSize);
            value = (char*) malloc(valueSize);
            clGetDeviceInfo(devices[j], CL_DEVICE_NAME, valueSize, value, NULL);
            printf("%d. Device: %s\n", j+1, value);
            free(value);

            // print hardware device version
            clGetDeviceInfo(devices[j], CL_DEVICE_VERSION, 0, NULL, &valueSize);
            value = (char*) malloc(valueSize);
            clGetDeviceInfo(devices[j], CL_DEVICE_VERSION, valueSize, value, NULL);
            printf(" %d.%d Hardware version: %s\n", j+1, 1, value);
            free(value);

            // print software driver version
            clGetDeviceInfo(devices[j], CL_DRIVER_VERSION, 0, NULL, &valueSize);
            value = (char*) malloc(valueSize);
            clGetDeviceInfo(devices[j], CL_DRIVER_VERSION, valueSize, value, NULL);
            printf(" %d.%d Software version: %s\n", j+1, 2, value);
            free(value);

            // print c version supported by compiler for device
            clGetDeviceInfo(devices[j], CL_DEVICE_OPENCL_C_VERSION, 0, NULL, &valueSize);
            value = (char*) malloc(valueSize);
            clGetDeviceInfo(devices[j], CL_DEVICE_OPENCL_C_VERSION, valueSize, value, NULL);
            printf(" %d.%d OpenCL C version: %s\n", j+1, 3, value);
            free(value);

            // print parallel compute units
            clGetDeviceInfo(devices[j], CL_DEVICE_MAX_COMPUTE_UNITS,
                    sizeof(maxComputeUnits), &maxComputeUnits, NULL);
            printf(" %d.%d Parallel compute units: %d\n", j+1, 4, maxComputeUnits);

        }

        free(devices);

    }

    free(platforms);
}