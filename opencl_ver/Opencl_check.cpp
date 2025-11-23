#include <iostream>
#include <CL/cl.h>

#define CHECK_ERROR(err) \
    if (err != CL_SUCCESS) { \
        std::cerr << "OpenCL Error: " << err << " at line " << __LINE__ << std::endl; \
        return -1; \
    }

int main() {
    cl_int err;
    cl_uint num_platforms;

    std::cout << "--- OpenCL Runtime Check ---\n";

    // 1. Get the number of available platforms
    err = clGetPlatformIDs(0, NULL, &num_platforms);
    CHECK_ERROR(err);

    if (num_platforms == 0) {
        std::cout << "SUCCESS: OpenCL is linked, but NO platforms (drivers) found. Check GPU drivers.\n";
        return 0;
    }

    std::cout << "Found " << num_platforms << " platform(s).\n";

    std::vector<cl_platform_id> platforms(num_platforms);
    err = clGetPlatformIDs(num_platforms, platforms.data(), NULL);
    CHECK_ERROR(err);

    for (cl_uint i = 0; i < num_platforms; ++i) {
        char buffer[1024];

        // 2. Print platform name
        clGetPlatformInfo(platforms[i], CL_PLATFORM_NAME, sizeof(buffer), buffer, NULL);
        std::cout << "[" << i << "] Platform Name: " << buffer << "\n";

        cl_uint num_devices;
        clGetDeviceIDs(platforms[i], CL_DEVICE_TYPE_ALL, 0, NULL, &num_devices);

        if (num_devices > 0) {
            std::vector<cl_device_id> devices(num_devices);
            clGetDeviceIDs(platforms[i], CL_DEVICE_TYPE_ALL, num_devices, devices.data(), NULL);
            std::cout << "    Found " << num_devices << " device(s):\n";

            for (cl_uint j = 0; j < num_devices; ++j) {
                // 3. Print device name
                clGetDeviceInfo(devices[j], CL_DEVICE_NAME, sizeof(buffer), buffer, NULL);
                std::cout << "      - Device Name: " << buffer << "\n";
            }
        }
    }

    std::cout << "----------------------------\n";
    std::cout << "SUCCESS: OpenCL is fully installed and linked!\n";

    return 0;
}