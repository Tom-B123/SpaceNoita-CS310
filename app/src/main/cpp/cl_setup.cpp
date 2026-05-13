#include "cl_setup.h"


CL::CL(buffer buf) {

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

// Check if an opencl function gave an error and exit.
void CL::check_error(cl_int err, std::string message) {

    if (err != CL_SUCCESS) {
        std::cerr << message << " Error code: " << err << std::endl;
        exit(1);
    }
}

// To Do::: This creates more and more buffer objects each time.
int CL::setArg(int arg_n, buffer buf,cl::Kernel kernel) {
    cl::Buffer mem_buffer(context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, buf.width * buf.height * sizeof(buf.data[0]),buf.data);
    mem_buffers.push_back(mem_buffer);

    cl_int err = kernel.setArg(arg_n, mem_buffer);
    check_error(err, "Failed to set buffer kernel argument!");

    return mem_buffers.size()-1;
}

void CL::setArg(int arg_n, int value,cl::Kernel kernel) {
    cl_int err = kernel.setArg(arg_n, value);
    check_error(err, "Failed to set integer kernel argument!");
}

void CL::enqueueKernel(int task_width, int task_height,cl::Kernel kernel) {
    cl_int err = command_queue.enqueueNDRangeKernel(kernel, cl::NullRange, cl::NDRange(task_width * task_height), cl::NullRange,nullptr,&task_finished);
    task_finished.wait();
}
void CL::enqueueReadBuffer(int task_width, int task_height, char* buffer, size_t buffer_index) {
    command_queue.enqueueReadBuffer(mem_buffers.at(buffer_index), CL_TRUE, 0, task_width * task_height * sizeof(buffer[0]), buffer,nullptr,&task_finished);
    task_finished.wait();
}
void CL::enqueueWriteBuffer(int task_width, int task_height, char* buffer, size_t buffer_index) {
    command_queue.enqueueWriteBuffer(mem_buffers.at(buffer_index), CL_TRUE, 0, task_width * task_height * sizeof(buffer[0]), buffer,nullptr,&task_finished);
    task_finished.wait();
}
void CL::free() {
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

std::string find_shader_file(std::string shader) {
    std::vector<std::string> search_paths = {
        "app/src/main/shaders/" + shader,
        "../app/src/main/shaders/" + shader,
        "../../app/src/main/shaders/" + shader,
        "../../../app/src/main/shaders/" + shader,
        "src/main/shaders/" + shader,
        "../src/main/shaders/" + shader,
        "" + shader,
        "./" + shader
    };

    for (const auto& path : search_paths) {
        std::ifstream test(path);
        if (test.is_open()) {
            test.close();
            return path;
        }
    }

    return "C:/Users/tomhb/University/cs310/FallingSandIter1/app/src/main/shaders/" + shader;
}

std::string get_shader_src(std::string shader) {
    std::ifstream hello_world_file(find_shader_file(shader));
    std::string src(std::istreambuf_iterator<char>(hello_world_file), (std::istreambuf_iterator<char>()));
    return src;
}


