#include "app.h"
#include "cl_setup.h"
#include "buffer.h"

/**
 *  Initialise the data buffer to be empty (" ")
 *  @param buf the buffer pointer
 *  @param width the width of the world
 *  @param height the height of the world
 */

void update(CL* cl_components, //cl::Kernel kernel, cl::CommandQueue queue, cl::Buffer mem_buf, 
        int* iteration, int update_count, int n_width, int n_height,
        buffer buf, int data_buffer) {

    for (int i = 0; i < update_count; i++) {
        cl_components->setArg(1,*iteration);
        // NDRange = num of parallel operations
        cl_components->enqueueNDRangeKernel(buf.width / n_width, buf.height / n_height);

        *iteration = (*iteration) + 1;
    }
    cl_components->enqueueReadBuffer(buf.width, buf.height,buf.data,data_buffer);
}
