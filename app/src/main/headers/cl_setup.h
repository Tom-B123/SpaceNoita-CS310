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
        /**
         *  Initialise OpenCL: 
         */
        CL(buffer buf) {
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
            

            std::string src = get_shader_src("process.cl");
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
            check_error(err,"Failed to build program!");

            // Create the queue, input / output buffer and kernel
            cl::CommandQueue n_queue(context,device);
            command_queue = n_queue;


            // Verify kernel exists

            process_kernel = cl::Kernel(program, "process", &err);
            check_error(err, "Failed to create the 'process' kernel!");
            render_kernel = cl::Kernel(program, "render", &err);
            check_error(err, "Failed to create the 'render' kernel!");
                
            cl::Event n_task_finished;
            task_finished = n_task_finished;

        }

        void check_error(cl_int err, std::string message) {
            if (err != CL_SUCCESS) {
                std::cerr << message << " Error code: " << err << std::endl;
                exit(1);
            }
        }
        /**
         *  Creates a new buffer and returns its index.
         */
        int setArg(int arg_n,buffer buf) {

            cl::Buffer mem_buffer(context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, buf.width * buf.height * sizeof(buf.data[0]),buf.data);
            mem_buffers.push_back(mem_buffer);

            cl_int err = process_kernel.setArg(arg_n, mem_buffer);
            check_error(err, "Failed to set buffer kernel argument!");

            return mem_buffers.size()-1;
        }
        void setArg(int arg_n, int value) {
            cl_int err = process_kernel.setArg(arg_n, value);
            check_error(err, "Failed to set integer kernel argument!");
        }

        void enqueueNDRangeKernel(int task_width, int task_height) {
            cl_int err = command_queue.enqueueNDRangeKernel(process_kernel, cl::NullRange, cl::NDRange(task_width * task_height * BUFFER_RUN), cl::NullRange,nullptr,&task_finished);
            task_finished.wait();
        }

        void enqueueReadBuffer(int task_width, int task_height,char* buffer,size_t buffer_index) {
            command_queue.enqueueReadBuffer(mem_buffers.at(buffer_index), CL_TRUE, 0, task_width * task_height * sizeof(buffer[0]), buffer,nullptr,&task_finished);
            task_finished.wait();
        }
        void enqueueWriteBuffer(int task_width, int task_height,char* buffer, size_t buffer_index) {
            command_queue.enqueueWriteBuffer(mem_buffers.at(buffer_index), CL_TRUE, 0, task_width * task_height * sizeof(buffer[0]), buffer,nullptr,&task_finished);
            task_finished.wait();
        }
        void enqueueWriteBuffer(int task_width, int task_height,int* buffer,size_t buffer_index) {
            command_queue.enqueueWriteBuffer(mem_buffers.at(buffer_index), CL_TRUE, 0, task_width * task_height * sizeof(buffer[0]), buffer,nullptr,&task_finished);
            task_finished.wait();
        }
        void free() {
            for (size_t i = 0; i < mem_buffers.size(); i++) {
                clReleaseMemObject(mem_buffers.at(i).get());
            }
            clReleaseCommandQueue(command_queue.get());
            clReleaseContext(context.get());
            clReleaseDevice(device.get());
            clReleaseEvent(task_finished.get());
            clReleaseKernel(process_kernel.get());
            clReleaseKernel(render_kernel.get());
            clReleaseProgram(program.get());
        }
};

#endif
