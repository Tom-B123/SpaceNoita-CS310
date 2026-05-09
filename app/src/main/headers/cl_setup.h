#ifndef CL_SETUP_H
#define CL_SETUP_H
// Functions to set up open CL.
#include "app.h"
#include "CL/opencl.hpp"

std::string find_shader_file(std::string shader); 
std::string get_shader_src(std::string shader); 

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
        CL(char* buf,int width, int height) {
            if (width%2 == 1) width++;
            if (height%2 == 1) height++;
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
            

            std::string src = get_shader_src("hello_world.cl");
            /**
             * Compile the program which will run on the device.
             * */
            cl::Program::Sources sources;
            sources.push_back({src.c_str(), src.length() + 1});

            // Create the context and program
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

            // Create the queue, input / output buffer and kernel
            cl::CommandQueue n_queue(context,device);
            commandQueue = n_queue;

            cl::Buffer n_mem_buffer(context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, width * height * sizeof(buf[0]),buf);
            mem_buffer = n_mem_buffer;

            // Verify kernel exists

            kernel = cl::Kernel(program, "helloWorld", &err);
            if (err != CL_SUCCESS) {
                std::cerr << "Failed to create kernel 'helloWorld'! Error: " << err << std::endl;

                // List available kernels for debugging
                std::vector<cl::Kernel> kernels;
                program.createKernels(&kernels);
                std::cout << "Available kernels in program: " << kernels.size() << std::endl;
                for (auto& k : kernels) {
                    std::cout << "  - " << k.getInfo<CL_KERNEL_FUNCTION_NAME>() << std::endl;
                }
                exit(1);
            }
            cl::Event n_task_finished;
            taskFinished = n_task_finished;

        }
        void setBufferArg(int arg_n) {
            cl_int err = kernel.setArg(arg_n, mem_buffer);
            if (err != CL_SUCCESS) {
                std::cout << "Failed to set kernel argument " << "mem buffer" << err << std::endl;
            }
        }
        void setArg(int arg_n, int value) {
            cl_int err = kernel.setArg(arg_n, value);
            if (err != CL_SUCCESS) {
                std::cout << "Failed to set kernel argument " << arg_n << err << std::endl;
            }
        }
        void enqueueNDRangeKernel(int task_width, int task_height) {

            cl_int err = commandQueue.enqueueNDRangeKernel(kernel, cl::NullRange, cl::NDRange(task_width * task_height), cl::NullRange,nullptr,&taskFinished);
            taskFinished.wait();
        }
        void enqueueReadBuffer(int task_width, int task_height,char* buffer) {
            commandQueue.enqueueReadBuffer(mem_buffer, CL_TRUE, 0, task_width * task_height * sizeof(buffer[0]), buffer,nullptr,&taskFinished);
            taskFinished.wait();
        }
        void enqueueWriteBuffer(int task_width, int task_height,char* buffer) {
            commandQueue.enqueueWriteBuffer(mem_buffer, CL_TRUE, 0, task_width * task_height * sizeof(buffer[0]), buffer,nullptr,&taskFinished);
            taskFinished.wait();
        }
};

#endif
