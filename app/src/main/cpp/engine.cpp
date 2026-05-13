#include "engine.h"
#include "buffer.h"
#include "chunk_manager.h"
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
    cl(n_cl),
    window(n_window),
    chunk_manager(ChunkManager(world_width,world_height,n_cl)),
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
    int iteration = 0;

    // Set Process kernel arguments
    cl->setArg(5, render_buf.width,cl->process_kernel);
    cl->setArg(6, render_buf.height,cl->process_kernel);
    cl->setArg(7, n_width,cl->process_kernel);
    cl->setArg(8, n_height,cl->process_kernel);

    // Set Render kernel arguments
    cl->setArg(6, render_buf.width,cl->render_kernel);
    cl->setArg(7, render_buf.height,cl->render_kernel);


    // Main loop, 
    char materials[] = {'S','O','W'};

    int* count = new int[256];

    while (window->is_open()) {

        chunk_manager.update_chunks(cl);

        render_buf = chunk_manager.get_render_buffer();

        window->update_texture(render_buf.data);

        window->refresh();
        window->draw();
    }

    window->close();

    Sleep(2000);

    delete[](render_buf.data);
    cl->free();

    return 0;
}
