#include "CL/cl.h"
#include "CL/opencl.hpp"
#include "GLFW/glfw3.h"
#include <chrono>
#include <iomanip>
#include <iostream>
#include <stdlib.h>
#include <thread>
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

class FPSCount {
    private:
        std::chrono::steady_clock::time_point last_time;
        double target_fps;
    public:
        FPSCount(double n_target_fps) : target_fps(n_target_fps){
            last_time = std::chrono::steady_clock::now();
        }
        void nextFrame() {
            auto new_time = std::chrono::steady_clock::now();
            
            double delta = std::chrono::duration_cast<std::chrono::milliseconds>(new_time - last_time).count();
            // std::cout << "\033[1;0HFPS: " << std::fixed << std::setprecision(1) << (1000.0/delta) << "     ";
            std::cout.flush();

            double expected_time = 1000.0 / target_fps;

            std::this_thread::sleep_for(std::chrono::milliseconds((long)(expected_time - delta)));
            last_time = new_time;
        }
};

void print_buf(int width, int height, char* buf) {
    std::cout << "\033[H";

    std::cout.write(buf,(width+3) * (height+2));
    std::cout << std::endl;
    
}

void draw(int width, int height, char* data) {
    print_buf(width,height,data);
}



/**
 *  Initialise the data buffer to be empty (" ")
 *  @param buf the buffer pointer
 *  @param width the width of the world
 *  @param height the height of the world
 */
char* init_buf(int width, int height) {

    char* buf = new char[width * height]();
    // draw(width,height,buf);

    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            buf[i*width + j] = ' ';
        }
    }

    return buf;
}

char* init_output_buf(int width, int height) {
    char* output_buf = new char[(width+3) * (height+2)]();

    char c;
    for (int i = 0; i < height+2; i++) {
        for (int j = 0; j < width+3; j++) {
            c = ' ';
            if (j == 0 || j == width + 1 ) {c = '|'; }
            if (i == 0 || i == height + 1) { c = '-'; }
            if (j == width + 2) {c = '\n'; }
            output_buf[i*(width+3) + j] = c;
        }
    }

    return output_buf;
}

void update(cl::Kernel kernel, cl::CommandQueue queue, cl::Buffer mem_buf, 
        int* iteration, int update_count, int width, int height, int n_width, int n_height,
        char* buf, char* output_buf) {
    
    cl::Event completeEvent;

    for (int i = 0; i < 5; i++) {
        kernel.setArg(1, *iteration);
        // NDRange = num of parallel operations
        queue.enqueueNDRangeKernel(kernel, cl::NullRange, cl::NDRange(width * height / n_width / n_height), cl::NullRange,nullptr,&completeEvent);
        completeEvent.wait();

        *iteration = (*iteration) + 1;
    }

    queue.enqueueReadBuffer(mem_buf, CL_TRUE, 0, width * height * sizeof(buf[0]), buf);

    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            output_buf[(i+1)*(width+3) + (j+1)] = buf[i*width + j];
        }
    }
}

int main_loop(cl::Program program, cl::Context context, cl::Device device,
        int width, int height, int n_width, int n_height) {
    // Fill the buffer of raw data and the output buffer, which has formatting to be displayed.
    char* buf = init_buf(width,height);
    char* output_buf = init_output_buf(width,height);


    // Tracks the simulation framerate
    FPSCount fps = FPSCount(120);


    // Create the memory buffer that will store world data. Potentially use more buffers to store larger world in chunks
    cl::Buffer mem_buf(context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, width * height * sizeof(buf[0]),buf);
    // Create the kernel program using the program "helloWorld"
    cl::Kernel kernel(program, "helloWorld", nullptr);

    /**
     * Set kernel argument.
     * */


    // Update n times per tick
    int update_count = 5;

    int iteration = 0;

    // Send data to the GPU.
    kernel.setArg(0, mem_buf);
    kernel.setArg(2, width);
    kernel.setArg(3, height);
    kernel.setArg(4, n_width);
    kernel.setArg(5, n_height);


    // Create the queue for queueing GPU tasks
    cl::CommandQueue queue(context, device);

    // Main loop, 
    while (1) {

        update(kernel,queue,mem_buf, &iteration, update_count, width, height, n_width, n_height, buf, output_buf);

        draw(width,height,output_buf);

        fps.nextFrame();
    }

    delete[](buf);
    delete[](output_buf);

    return 0;
}

int main(){


    GLFWwindow* window;

    /* Initialize the library */
    if (!glfwInit())
        return -1;

    /* Create a windowed mode window and its OpenGL context */
    window = glfwCreateWindow(640, 480, "Hello World", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        return -1;
    }

    /* Make the window's context current */
    glfwMakeContextCurrent(window);

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

    const int width = 32;
    const int n_width = 2;
    const int height = 10;
    const int n_height = 2;
    
    int error = main_loop(program,context,device,width,height,n_width,n_height);

    glfwTerminate();

    return error;
}
