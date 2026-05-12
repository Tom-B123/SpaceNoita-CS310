#include "engine.h"
#include "graphics.h"

// void update(CL* cl_components, //cl::Kernel kernel, cl::CommandQueue queue, cl::Buffer mem_buf, 
//         int* iteration, int update_count, int n_width, int n_height,
//         buffer buf, int data_buffer) {
//
//     for (int i = 0; i < update_count; i++) {
//         cl_components->setArg(1,*iteration);
//         // NDRange = num of parallel operations
//         cl_components->enqueueNDRangeKernel(buf.width / n_width, buf.height / n_height);
//
//         *iteration = (*iteration) + 1;
//     }
//     cl_components->enqueueReadBuffer(buf.width, buf.height,buf.data,data_buffer);
// }
Engine::Engine(int world_width, int world_height,
        CL* n_cl, GameWindow* n_window) : 
    chunk_manager(ChunkManager(world_width,world_height)),
    window(n_window),
    cl(n_cl),
    iteration(0)
{
    n_width = 2;
    n_height = 2;

}

void Engine::update() {

}

int Engine::main_loop() {
    buffer render_buf = chunk_manager.get_render_buffer();

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


    // Main loop, 
    char materials[] = {'S','O','W'};


    cl->enqueueWriteBuffer(render_buf.width, render_buf.height, render_buf.data,data_buffer);
    cl->enqueueWriteBuffer(spawners.width, spawners.height, spawners.data,spawner_buffer);

    int* count = new int[256];

    while (window->is_open()) {

        // update(&cl_components, &iteration, update_count, n_width, n_height,
        //         buf,data_buffer);
        chunk_manager.update_chunk();

        render_buf = chunk_manager.get_render_buffer();

        window->update_texture(render_buf.data);

        window->refresh();
        window->draw();
    }

    window->close();

    Sleep(2000);

    delete[](render_buf.data);
    delete[](spawners.data);
    cl->free();

    return 0;
}
