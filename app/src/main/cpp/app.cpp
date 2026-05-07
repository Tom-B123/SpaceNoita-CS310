#include "CL/cl.h"
#include "CL/opencl.hpp"
#include <chrono>
#include <iomanip>
#include <iostream>
#include <stdlib.h>
#include<vector>
#include <fstream>
#include<conio.h>
#include <stdio.h>
#include <Windows.h>
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

void print_buf(int width, int height, char* buf) {
    std::cout << "\033[H";

    std::cout.write(buf,(width+3) * (height+2));
    std::cout << std::endl;
    
}

class FPSCount {
    private:
        std::chrono::steady_clock::time_point last_time;
    public:
        FPSCount() {
            last_time = std::chrono::steady_clock::now();
        }
        void nextFrame() {
            auto new_time = std::chrono::steady_clock::now();
            
            double delta = std::chrono::duration_cast<std::chrono::milliseconds>(new_time - last_time).count();
            std::cout << "\033[1;0HFPS: " << std::fixed << std::setprecision(1) << (1000.0/delta) << "     ";
            std::cout.flush();

            last_time = new_time;
        }
};

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

    const int width = 700;
    const int n_width = 2;
    const int height = 212;
    const int n_height = 2;

    FPSCount fps = FPSCount();

    // nxn world
    char* buf = new char[width * height]();
    char* output_buf = new char[(width+3) * (height+2)]();

    for (int i = 0; i < width + 2; i++) {
        output_buf[i] = '-';
        output_buf[(width+3) * (height+1) + i] = '-';
    }

    for (int i = 0; i < height + 2; i++) {
        output_buf[(width + 3) * i + width + 2] = '\n';
        
        if (i == 0 || i == height + 1) continue;
        
        output_buf[(width + 3) * i] = '|';
        output_buf[(width + 3) * i + width + 1] = '|';
    }

    output_buf[(width+3) * (height+2) - 1] = '\0';



    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            buf[i*width + j] = ' ';
            output_buf[(i+1)*(width+3) + (j+1)] = ' ';
        }
    }

    cl::Buffer memBuf(context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, width * height * sizeof(buf[0]),buf);
    cl::Kernel kernel(program, "helloWorld", nullptr);

    /**
     * Set kernel argument.
     * */


    int n_spawners = 1;
    int* spawners;
    spawners = new int {1};

    int iteration = 0;
    kernel.setArg(0, memBuf);
    kernel.setArg(2, width);
    kernel.setArg(3, height);
    kernel.setArg(4, n_width);
    kernel.setArg(5, n_height);
    // kernel.setArg(6, n_spawners);
    // kernel.setArg(7, spawners);

    /**
     * Run the kernel function and collect its result.
     * */

    cl::CommandQueue queue(context, device);
    cl::Event completeEvent;

    while (1) {
        // buf[4] = '@';
        // queue.enqueueWriteBuffer(memBuf,CL_TRUE,0,sizeof(buf),buf);
        for (int i = 0; i < 100; i++) {
            kernel.setArg(1, iteration);
            // NDRange = num of parallel operations
            queue.enqueueNDRangeKernel(kernel, cl::NullRange, cl::NDRange(width * height / n_width / n_height), cl::NullRange,nullptr,&completeEvent);
            completeEvent.wait();

            iteration++;
        }


        queue.enqueueReadBuffer(memBuf, CL_TRUE, 0, width * height * sizeof(buf[0]), buf);

        for (int i = 0; i < height; i++) {
            for (int j = 0; j < width; j++) {
                output_buf[(i+1)*(width+3) + (j+1)] = buf[i*width + j];
            }
        }


        print_buf(width,height,output_buf);

        fps.nextFrame();

    }

    return 0;
}
