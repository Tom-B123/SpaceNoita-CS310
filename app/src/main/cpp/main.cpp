#include "app.h"
#include "graphics.h"
#include "cl_setup.h"
#include "engine.h"


int main_loop(CL& cl_components,
        int width, int height, int n_width, int n_height,
        char* buf, char* output_buf,GameWindow& window) {

    // Tracks the simulation framerate
    FPSCount fps = FPSCount(120);

    // Update n times per tick
    int update_count = 10;

    int iteration = 0;

    char* spawners = init_buf(width,height,0);

    // Send data to the GPU.
    int data_buffer = cl_components.setArg(0,buf,width,height);
    cl_components.setArg(2, width);
    cl_components.setArg(3, height);
    cl_components.setArg(4, n_width);
    cl_components.setArg(5, n_height);
    int spawner_buffer = cl_components.setArg(6, spawners,width,height);
    // Create the queue for queueing GPU tasks

    // Main loop, 
    //

    int plinko_count = 10;
    for (int j = 0; j < plinko_count; j++) {
        for (int i = width / 6; i < 5 * width / 6; i+=2) {
            buf[width * ((2*j) + height/2) + i + j%2] = 'R';
            // buf[width * (plinko_count + 10 + j+height/2) + i/2] = 'R';
        }
    }

    char materials[] = {'S','O','W'};

    for (int i = 0; i < width; i+=50) {
        spawners[(1 * height / 6) * width + i] = materials[(i/5) % 3];
    }

    cl_components.enqueueWriteBuffer(width, height, buf,data_buffer);
    cl_components.enqueueWriteBuffer(width, height, spawners,spawner_buffer);

    // cl_components.enqueueWriteBuffer(4, 1, spawners);
    int* count = new int[256];

    while (window.is_open()) {

        update(&cl_components, &iteration, update_count, width, height, n_width, n_height,
                buf,output_buf,data_buffer);

        window.updateTexture(buf);

        window.refresh();
        window.draw();

        fps.nextFrame();
    }

    Sleep(300);

    delete[](buf);
    delete[](output_buf);
    delete[](spawners);
    window.close();
    // cl_components.free();

    return 0;
}

int main(){

    GameWindow window(WORLD_WIDTH,WORLD_HEIGHT,PIXEL_SCALE);

    char* buf = init_buf(WORLD_WIDTH, WORLD_HEIGHT,' ');
    char* output_buf = init_output_buf(WORLD_WIDTH, WORLD_HEIGHT);

    CL cl_components(buf,WORLD_WIDTH,WORLD_HEIGHT);
    
    int error = main_loop(cl_components,WORLD_WIDTH,WORLD_HEIGHT,2,2,
            buf,output_buf,window);

    return error;
}
