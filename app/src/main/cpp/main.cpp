#include "app.h"
#include "graphics.h"
#include "cl_setup.h"
#include "engine.h"
#include "buffer.h"


int main_loop(CL& cl_components,int n_width, int n_height,
        buffer buf,GameWindow& window) {

    // Tracks the simulation framerate
    FPSCount fps = FPSCount(120);

    // Update n times per tick
    int update_count = 10;

    int iteration = 0;

    buffer spawners = init_buf(WORLD_WIDTH,WORLD_HEIGHT,(char)0);

    // Send data to the GPU.
    int data_buffer = cl_components.setArg(0,buf);
    cl_components.setArg(2, buf.width);
    cl_components.setArg(3, buf.height);
    cl_components.setArg(4, n_width);
    cl_components.setArg(5, n_height);
    int spawner_buffer = cl_components.setArg(6, spawners);

    int width = WORLD_WIDTH;
    int height = WORLD_HEIGHT;
    // Create the queue for queueing GPU tasks

    // Main loop, 
    //

    int plinko_count = 0;
    for (int j = 0; j < plinko_count; j++) {
        for (int i = width / 6; i < 5 * width / 6; i+=2) {
            // set_buffer(width * ((2*j) + height/2) + i + j%2,'R');
        }
    }

    // buf[(height-1) * width + width-1] = 'S';
    // buf[(height-2) * width + width-1] = 'S';

    char materials[] = {'S','O','W'};

    // for (int i = width / 2; i < width; i+=width) {
        // spawners[(3 * height / 6) * width + i] = materials[(i/5) % 3];
    // }

    cl_components.enqueueWriteBuffer(buf.width, buf.height, buf.data,data_buffer);
    cl_components.enqueueWriteBuffer(spawners.width, spawners.height, spawners.data,spawner_buffer);

    int* count = new int[256];

    while (window.is_open()) {

        update(&cl_components, &iteration, update_count, n_width, n_height,
                buf,data_buffer);

        window.updateTexture(buf.data);

        window.refresh();
        window.draw();

        buf.data[2] = 'S';
        cl_components.enqueueWriteBuffer(buf.width, buf.height, buf.data,data_buffer);

        fps.nextFrame();
    }

    window.close();

    Sleep(2000);

    delete[](buf.data);
    delete[](spawners.data);
    cl_components.free();

    return 0;
}

int main(){

    // int width = 64;
    // int height = 64;
    // int* qt_buf = init_int_buf(width,height, 0);
    //
    // qt_buf[16 * width + 16] = 1;
    //
    // QuadTree qt = QuadTree(64,64,qt_buf);
    //
    // return 0;

    GameWindow window(WORLD_WIDTH,WORLD_HEIGHT,PIXEL_SCALE);

    buffer buf = init_buf(WORLD_WIDTH, WORLD_HEIGHT,' ');

    CL cl_components(buf);
    
    int error = main_loop(cl_components,2,2,
            buf,window);

    return error;
}
