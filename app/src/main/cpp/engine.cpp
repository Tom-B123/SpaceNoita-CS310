#include "app.h"
#include "cl_setup.h"
#include "engine.h"
#include "graphics.h"

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
void update(CL* cl_components, //cl::Kernel kernel, cl::CommandQueue queue, cl::Buffer mem_buf, 
        int* iteration, int update_count, int width, int height, int n_width, int n_height,
        char* buf, char* output_buf) {
    

    for (int i = 0; i < update_count; i++) {
        cl_components->setArg(1,*iteration);
        // NDRange = num of parallel operations
        cl_components->enqueueNDRangeKernel(width / n_width, height / n_height);

        *iteration = (*iteration) + 1;
    }
    // std::cout << "Before: " << buf << std::endl;
    cl_components->enqueueReadBuffer(width, height,buf);
    // std::cout << "After: " << buf << std::endl;
    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            output_buf[(i+1)*(width+3) + (j+1)] = buf[i*width + j];
        }
    }

}
