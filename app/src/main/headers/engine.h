#ifndef ENGINE_H
#define ENGINE_H
// Handle the game update loop for the player and the world
#include "app.h"
#include "chunk_manager.h"
#include "cl_setup.h"
#include "graphics.h"

// Represents the position of the camera with respect to 0,0.
class Engine {
    private:
        ChunkManager chunk_manager;
        CL* cl;
        GameWindow* window;
        // Current simulation step
        int iteration;

        // Neighbourhood size, default is 2x2
        int n_width;
        int n_height;


    public:
        Engine(int world_width, int world_height, 
                CL* n_cl,GameWindow* n_window) : 
                chunk_manager(ChunkManager(world_width,world_height)),
                window(n_window),
                cl(n_cl),
                iteration(0)
        {
            n_width = 2;
            n_height = 2;

        }
        void update() {

        }

        int main_loop() {

            buffer render_buf = chunk_manager.get_render_buffer();

            // Tracks the simulation framerate
            FPSCount fps = FPSCount(120);

            // Update n times per tick
            int update_count = 10;

            int iteration = 0;

            buffer spawners = init_buf(WORLD_WIDTH,WORLD_HEIGHT,(char)0);

            // Send data to the GPU.
            int data_buffer = cl->setArg(0,render_buf);
            cl->setArg(2, render_buf.width);
            cl->setArg(3, render_buf.height);
            cl->setArg(4, n_width);
            cl->setArg(5, n_height);
            int spawner_buffer = cl->setArg(6, spawners);

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

            cl->enqueueWriteBuffer(render_buf.width, render_buf.height, render_buf.data,data_buffer);
            cl->enqueueWriteBuffer(spawners.width, spawners.height, spawners.data,spawner_buffer);

            int* count = new int[256];

            while (window->is_open()) {

                // update(&cl_components, &iteration, update_count, n_width, n_height,
                //         buf,data_buffer);

                window->updateTexture(render_buf.data);

                window->refresh();
                window->draw();

                // set_buffer(buf,3,0, {3,0,'S'} );
                // cl_components.enqueueWriteBuffer(buf.width, buf.height, buf.data,data_buffer);

                fps.nextFrame();
            }

            window->close();

            Sleep(2000);

            delete[](render_buf.data);
            delete[](spawners.data);
            cl->free();

            return 0;
        }
};

#endif
