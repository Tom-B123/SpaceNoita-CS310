#include "chunk_manager.h"

ChunkManager::ChunkManager(int n_world_width, int n_world_height) :
    world_width(n_world_width), 
    world_height(n_world_height)
{
    char* data = new char[world_width * world_height];
    for (int y = 0; y < world_height; y++) {
        for (int x = 0; x < world_width; x++) {
            data[y * world_width + x] = 'W';
        }
    }
    render_buffer = {world_width,world_height,data};
    chunks = { Chunk(0,0,chunk_size) };
}

buffer ChunkManager::get_render_buffer() {
    return render_buffer;
}

void ChunkManager::render_chunk(CL* cl,int render_buffer_index) {
    Chunk chunk = chunks.at(0);
    int chunk_buffer_index = cl->setArg(0,chunk.get_data(),cl->render_kernel);
    cl->enqueueWriteBuffer(chunk_size, chunk_size, chunk.get_data().data, chunk_buffer_index);
    cl->enqueueKernel(chunk_size,chunk_size,cl->render_kernel);
    cl->enqueueReadBuffer(world_width,world_height, render_buffer.data, render_buffer_index);
}

void ChunkManager::update_chunk(CL* cl) {

}
