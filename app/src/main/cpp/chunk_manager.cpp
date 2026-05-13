#include "chunk_manager.h"
#include "buffer.h"
#include "chunk.h"

ChunkManager::ChunkManager(int n_world_width, int n_world_height, CL* cl) :
    world_width(n_world_width), 
    world_height(n_world_height)
{
    camera = {0,0};

    chunk_size = 128;

    render_buffer = init_render_buf(world_width,world_height,'?');
    chunks = { 
        Chunk(0,0,chunk_size,cl),
        Chunk(2,0,chunk_size,cl) 
    };

    chunks.at(1).get_data().data[3] = 'S';

    // Tell the kernel what size each chunk is for rendering
    cl->setArg(3,chunk_size,cl->render_kernel);
    cl->setArg(4,chunk_size,cl->process_kernel);

    // Create the GPU buffer for rendering and link it to the render kernel
    render_buffer_index = cl->makeRenderBuffer(render_buffer);
    cl->setArg(8,render_buffer,render_buffer_index,cl->render_kernel);
    // Link the render buffer to the process kernel too
    // cl->setArg(0, render_buffer,render_buffer_index,cl->process_kernel);
}

buffer ChunkManager::get_render_buffer() {
    return render_buffer;
}

void ChunkManager::render_chunk(CL* cl,Chunk chunk) {
    // Tell the kernel the chunk's data and the chunk's position
    cl->setArg(0,chunk.get_data(),chunk.buffer_index,cl->render_kernel);
    cl->setArg(1,chunk.chunk_x,cl->render_kernel);
    cl->setArg(2,chunk.chunk_y,cl->render_kernel);

    cl->setArg(4,camera.x,cl->render_kernel);
    cl->setArg(5,camera.y,cl->render_kernel);

    cl->enqueueWriteBuffer(chunk_size, chunk_size, chunk.get_data().data, chunk.buffer_index);
    cl->enqueueKernel(chunk_size,chunk_size,cl->render_kernel);
    cl->enqueueRenderReadBuffer(world_width,world_height, render_buffer.data, render_buffer_index);
}

void ChunkManager::update_chunks(CL* cl,int& iteration) {
    int c_iteration = 0;
    for (int i = 0; i < chunks.size(); i++) {
        c_iteration = iteration;
        Chunk chunk = chunks.at(i);

        char materials[] = {'S','W','R',' '};

        chunk.set_cell(chunk_size / 2,i * 4, 'W');
        refresh_chunk(cl,chunk);
        
        cl->setArg(0,chunk.get_data(),chunk.buffer_index,cl->process_kernel);
        cl->setArg(2,chunk.chunk_x,cl->process_kernel);
        cl->setArg(3,chunk.chunk_y,cl->process_kernel);

        for (int i = 0; i < chunk.highest_speed; i++) {
            // Create chunk_size/2 x chunk_size/2 tasks, each processing a 2x2 block.
            cl->setArg(1,c_iteration,cl->process_kernel);
            cl->enqueueKernel(chunk_size / 2, chunk_size / 2, cl->process_kernel);
            c_iteration++;
            // if (iteration%50 == 0) camera.x++;
        }
        cl->enqueueReadBuffer(chunk_size,chunk_size, chunk.get_data().data, chunk.buffer_index);
        render_chunk(cl,chunk);
    }
    iteration = c_iteration;
}

void ChunkManager::refresh_chunk(CL* cl,Chunk chunk) {
    cl->enqueueWriteBuffer(chunk_size, chunk_size, chunk.get_data().data, chunk.buffer_index);
}
