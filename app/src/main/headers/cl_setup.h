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
        cl::CommandQueue command_queue;
        cl::Event task_finished;
        cl::Context context;
        cl::Program program;
        std::vector<cl::Buffer> mem_buffers;

    public:
        // Public so we can use these when setting args / running programs
        cl::Kernel process_kernel;
        cl::Kernel render_kernel;

        CL();
        ~CL() {
            if (command_queue.get()) {
                command_queue.finish();
                clFinish(command_queue.get());
            }

            Sleep(100);
            mem_buffers.clear();
            std::cout << "CL freed" << std::endl;
            Sleep(100);

        }
        void check_error(cl_int err, std::string message);
        /**
         *  Make a new CL::Buffer
         *  @param buf the buffer struct containing the width, height and raw data
         *  @param run the size of each buffer element. Defult is BUFFER_RUN
         */
        int makeBuffer(buffer buf);
        int makeRenderBuffer(buffer buf);
        void setArg(int arg_n,buffer buf,int buffer_index, cl::Kernel kernel);
        void setArg(int arg_n, int value,cl::Kernel kernel);
        void enqueueKernel(int task_width, int task_height,cl::Kernel kernel);
        void enqueueReadBuffer(int task_width, int task_height,char* buffer,size_t buffer_index);
        void enqueueWriteBuffer(int task_width, int task_height,char* buffer, size_t buffer_index);
        void enqueueRenderReadBuffer(int task_width, int task_height, char* buffer, size_t buffer_index);
        void enqueueRenderWriteBuffer(int task_width, int task_height, char* buffer, size_t buffer_index);
        void free();
};

#endif
