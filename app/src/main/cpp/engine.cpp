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

    // Send data to the GPU.
    int render_buffer_index = cl->makeBuffer(render_buf);
    cl->setArg(0, render_buf,render_buffer_index,cl->process_kernel);
    cl->setArg(2, render_buf.width,cl->process_kernel);
    cl->setArg(3, render_buf.height,cl->process_kernel);
    cl->setArg(4, n_width,cl->process_kernel);
    cl->setArg(5, n_height,cl->process_kernel);


    // Main loop, 
    char materials[] = {'S','O','W'};

    cl->enqueueWriteBuffer(render_buf.width, render_buf.height, render_buf.data,render_buffer_index);
    cl->setArg(1,render_buf,render_buffer_index,cl->render_kernel);
    
    int* count = new int[256];

    while (window->is_open()) {

        // update(&cl_components, &iteration, update_count, n_width, n_height,
        //         buf,data_buffer);
        chunk_manager.update_chunk(cl,iteration);

        chunk_manager.render_chunk(cl,render_buffer_index);

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
