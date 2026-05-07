#ifndef CL_SETUP_H
#define CL_SETUP_H
// Functions to set up open CL.
#include "app.h"
#include "CL/opencl.hpp"

class CL {
    private:
        cl::Platform platform;
        cl::Device device;
        cl::Kernel kernel;
        cl::CommandQueue commandQueue;
        cl::Buffer mem_buffer;
        cl::Event taskFinished;
        cl::Context context;
        cl::Program program;
    public:
        /**
         *  Initialise OpenCL: 
         */
        CL(char* buf) {
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

            platform = platforms.front();
            std::vector<cl::Device> devices;
            platform.getDevices(CL_DEVICE_TYPE_ALL, &devices);
            if (devices.empty()){
                std::cerr << "No devices found!" << std::endl;
                exit(1);
            }

            /**
             * Return the first device found.
             **/

            device = devices.front();
            
            std::ifstream hello_world_file("app/src/main/cpp/hello_world.cl");
            std::string src(std::istreambuf_iterator<char>(hello_world_file), (std::istreambuf_iterator<char>()));

            /**
             * Compile the program which will run on the device.
             * */
            cl::Program::Sources sources;
            sources.push_back({src.c_str(), src.length() + 1});

            cl::Context n_context(device);
            context = n_context;
            cl::Program n_program(context, sources);
            program = n_program;

            auto err = program.build();
            if(err != CL_BUILD_SUCCESS){
                std::cerr << "Build Status: " << program.getBuildInfo<CL_PROGRAM_BUILD_STATUS>(device) 
                    << "Build Log:\t " << program.getBuildInfo<CL_PROGRAM_BUILD_LOG>(device) << std::endl;
                exit(1);
            }
            cl::CommandQueue n_queue(context,device);
            commandQueue = n_queue;

            cl::Buffer n_mem_buffer(context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, WORLD_WIDTH * WORLD_HEIGHT * sizeof(buf[0]),buf);
            mem_buffer = n_mem_buffer;
                
            cl::Kernel n_kernel(program, "helloWorld", nullptr);
            kernel = n_kernel;

        }
        void setBufferArg(int arg_n) {
            kernel.setArg(arg_n, mem_buffer);
        }
        void setArg(int arg_n, int value) {
            kernel.setArg(arg_n,value);
        }
        void enqueueNDRangeKernel(int task_width, int task_height) {
            commandQueue.enqueueNDRangeKernel(kernel, cl::NullRange, cl::NDRange(task_width * task_height), cl::NullRange,nullptr,&taskFinished);
            taskFinished.wait();
        }
        void enqueueReadBuffer(int task_width, int task_height,char* buffer) {
            commandQueue.enqueueReadBuffer(mem_buffer, CL_TRUE, 0, task_width * task_height * sizeof(buffer[0]), buffer);
        }
};

#endif
