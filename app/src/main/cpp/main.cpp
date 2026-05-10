#include "app.h"
#include "graphics.h"
#include "cl_setup.h"
#include "engine.h"


int main_loop(CL cl_components,
        int width, int height, int n_width, int n_height,
        char* buf, char* output_buf,GameWindow window) {

    // Tracks the simulation framerate
    FPSCount fps = FPSCount(120);


    // Update n times per tick
    int update_count = 250;

    int iteration = 0;

    char* spawners = new char[width * height];
    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            spawners[i * width + j] = 0;
        }
    }

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

    for (int i = width / 6; i < 5 * width / 6; i+=2) {
        buf[width * (height/2) + i] = 'R';
        buf[width * (2+height/2) + i+1] = 'R';
        buf[width * (4+height/2) + i] = 'R';
        buf[width * (6+height/2) + i+1] = 'R';
        buf[width * (8+height/2) + i] = 'R';
        buf[width * (15+height/2) + i/2] = 'R';
    }

    // spawners[(width / 4) % width] = 'S';
    spawners[((width / 2) + (width / 4)) % width] = 'W';
    spawners[width / 2] = 'O';
    spawners[width / 3] = 'O';

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
    
        for (int i = 0; i < 256; i++) {
            count[i] = 0;
        }
        for (int i = 0; i < width * height; i++) {
            count[buf[i]] ++;
        }
        for (int i = 0; i < 256; i++) {
            if (count[i] > 0) std::cout << (char)i << ": " << count[i] << std::endl;
        }

    }

    delete[](buf);
    delete[](output_buf);
    window.close();

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
