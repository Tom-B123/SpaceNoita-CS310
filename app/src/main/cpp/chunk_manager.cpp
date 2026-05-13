#include "chunk_manager.h"

ChunkManager::ChunkManager(int n_world_width, int n_world_height, CL* cl) :
    world_width(n_world_width), 
    world_height(n_world_height)
    // chunk_size(n_world_width * 2)
{
    char* data = new char[world_width * world_height];
    for (int y = 0; y < world_height; y++) {
        for (int x = 0; x < world_width; x++) {
            data[y * world_width + x] = '?';
        }
    }

    render_buffer = {world_width,world_height,data};
    chunks = { Chunk(0,0,chunk_size,cl) };
}

buffer ChunkManager::get_render_buffer() {
    return render_buffer;
}

void ChunkManager::render_chunk(CL* cl,int render_buffer_index) {
    Chunk chunk = chunks.at(0);
    cl->setArg(0,chunk.get_data(),chunk.buffer_index,cl->render_kernel);
    cl->enqueueWriteBuffer(chunk_size, chunk_size, chunk.get_data().data, chunk.buffer_index);
    cl->enqueueKernel(chunk_size,chunk_size,cl->render_kernel);
    cl->enqueueReadBuffer(world_width,world_height, render_buffer.data, render_buffer_index);
}

void ChunkManager::update_chunk(CL* cl,int& iteration) {
    Chunk chunk = chunks.at(0);

    char materials[] = {'S','W','R',' '};

    chunk.set_cell(iteration % chunk_size, (iteration / chunk_size) % chunk_size, materials[(iteration / chunk_size / chunk_size) % 4]);
    refresh_chunk(cl);

    cl->setArg(0,chunk.get_data(),chunk.buffer_index,cl->process_kernel);

    for (int i = 0; i < chunk.highest_speed; i++) {
        cl->setArg(1,iteration,cl->process_kernel);
        cl->enqueueKernel(chunk_size / 2, chunk_size / 2, cl->process_kernel);
        iteration++;
    }
    cl->enqueueReadBuffer(chunk_size,chunk_size, chunk.get_data().data, chunk.buffer_index);
}

void ChunkManager::refresh_chunk(CL* cl) {
    Chunk chunk = chunks.at(0);
    cl->enqueueWriteBuffer(chunk_size, chunk_size, chunk.get_data().data, chunk.buffer_index);
}
