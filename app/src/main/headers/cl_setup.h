#ifndef CL_SETUP_H
#define CL_SETUP_H
// Functions to set up open CL.
#include "CL/cl.h"
#include "app.h"
#include "CL/opencl.hpp"
#include "buffer.h"
#include <cstddef>
#include <vector>
std::string find_shader_file(std::string shader); 
std::string get_shader_src(std::string shader); 

class CL {
    private:
        cl::Platform platform;
        cl::Device device;
        cl::Kernel process_kernel;
        cl::Kernel render_kernel;
        cl::CommandQueue command_queue;
        cl::Event task_finished;
        cl::Context context;
        cl::Program program;
        std::vector<cl::Buffer> mem_buffers;

    public:
        CL(buffer buf);
        void check_error(cl_int err, std::string message);
        int setArg(int arg_n,buffer buf);
        void setArg(int arg_n, int value);
        void enqueueNDRangeKernel(int task_width, int task_height);
        void enqueueReadBuffer(int task_width, int task_height,char* buffer,size_t buffer_index);
        void enqueueWriteBuffer(int task_width, int task_height,char* buffer, size_t buffer_index);
        void free();
};

#endif
