#include "chunk_manager.h"

ChunkManager::ChunkManager(int world_width, int world_height) {
    char* data = new char[world_width * world_height];
    for (int y = 0; y < world_height; y++) {
        for (int x = 0; x < world_width; x++) {
            data[y * world_width + x] = 'S';
        }
    }
    render_buffer = {world_width,world_height,data};
}

buffer ChunkManager::get_render_buffer() {
    return render_buffer;
}

void ChunkManager::render_chunk() {

}

void ChunkManager::update_chunk() {

}
