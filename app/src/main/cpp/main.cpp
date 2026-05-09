#include "app.h"
#include "graphics.h"
#include "cl_setup.h"
#include "engine.h"
#include "buffer.h"


int main_loop(CL cl_components,
        int width, int height, int n_width, int n_height,
        char* buf, char* output_buf,GameWindow window) {

    // Tracks the simulation framerate
    FPSCount fps = FPSCount(120);


    // Update n times per tick
    int update_count = 4;

    int iteration = 0;

    // Send data to the GPU.
    cl_components.setBufferArg(0);
    cl_components.setArg(2, width);
    cl_components.setArg(3, height);
    cl_components.setArg(4, n_width);
    cl_components.setArg(5, n_height);



    // Create the queue for queueing GPU tasks

    // Main loop, 
    //

    while (window.is_open()) {

        update(&cl_components, &iteration, update_count, width, height, n_width, n_height,
                buf,output_buf);

        window.updateTexture(buf);

        window.refresh();
        window.draw();

        fps.nextFrame();

        buf[(30/10) % width] = 'S';
        buf[((width / 2) + (30/10)) % width] = 'W';
        cl_components.enqueueWriteBuffer(width, height, buf);
    }

    delete[](buf);
    delete[](output_buf);
    window.close();

    return 0;
}

int main(){

    GameWindow window(WORLD_WIDTH,WORLD_HEIGHT,PIXEL_SCALE);

    char* buf = init_buf(WORLD_WIDTH, WORLD_HEIGHT);
    char* output_buf = init_output_buf(WORLD_WIDTH, WORLD_HEIGHT);

    CL cl_components(buf,WORLD_WIDTH,WORLD_HEIGHT);
    
    int error = main_loop(cl_components,WORLD_WIDTH,WORLD_HEIGHT,2,2,
            buf,output_buf,window);


    return error;
}
