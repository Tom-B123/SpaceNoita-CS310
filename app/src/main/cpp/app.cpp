#include "CL/cl.h"
#include "CL/opencl.hpp"
#include <iostream>
#include <stdlib.h>
#include<vector>
#include <fstream>
#include"app.h"

cl::Device get_default_device(){
    
    /**
     * Search for all the OpenCL platforms available and check
     * if there are any.
     * */

    std::vector<cl::Platform> platforms;
    cl::Platform::get(&platforms);

    if (platforms.empty()){
        std::cerr << "No platforms found!" << std::endl;
        exit(1);
    }

    /**
     * Search for all the devices on the first platform and check if
     * there are any available.
     * */

    auto platform = platforms.front();
    std::vector<cl::Device> devices;
    platform.getDevices(CL_DEVICE_TYPE_ALL, &devices);

    if (devices.empty()){
        std::cerr << "No devices found!" << std::endl;
        exit(1);
    }

    /**
     * Return the first device found.
     * */

    return devices.front();
}

int main(){

    /**
     * Select a device.
     * */

    auto device = get_default_device();

    /**
     * Read OpenCL kernel file as a string.
     * */

    std::ifstream hello_world_file("app/src/main/cpp/hello_world.cl");
    std::string src(std::istreambuf_iterator<char>(hello_world_file), (std::istreambuf_iterator<char>()));
    
    /**
     * Compile the program which will run on the device.
     * */
    cl::Program::Sources sources;
    sources.push_back({src.c_str(), src.length() + 1});

    cl::Context context(device);
    cl::Program program(context, sources);

    auto err = program.build();
    if(err != CL_BUILD_SUCCESS){
        std::cerr << "Build Status: " << program.getBuildInfo<CL_PROGRAM_BUILD_STATUS>(device) 
        << "Build Log:\t " << program.getBuildInfo<CL_PROGRAM_BUILD_LOG>(device) << std::endl;
        exit(1);
    }
    
    /**
     * Create buffers and allocate memory on the device.
     * */

    const int width = 16;
    const int height = 16;

    // 16x16 world
    char buf[width * height];


    char c = ' ';
    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            if ( j == 5) c = '@';
            else c = ' ';

            buf[j * width + i] = c;
        }
    }
    buf[0] = 'T';

    cl::Buffer memBuf(context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, sizeof(buf),buf);
    cl::Kernel kernel(program, "helloWorld", nullptr);

    /**
     * Set kernel argument.
     * */

    kernel.setArg(0, memBuf);

    /**
     * Run the kernel function and collect its result.
     * */

    cl::CommandQueue queue(context, device);


    // NDRange = num of parallel operations
    queue.enqueueNDRangeKernel(kernel, cl::NullRange, cl::NDRange(width * height), cl::NullRange);
    queue.enqueueReadBuffer(memBuf, CL_TRUE, 0, sizeof(buf), buf);

    /**
     * Print result.
     * */

    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            std::cout << buf[i * width + j];
        }
        std::cout << std::endl;
    }
    return 0;
}
