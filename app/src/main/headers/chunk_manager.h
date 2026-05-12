#ifndef CHUNK_MANAGER_H
#define CHUNK_MANAGER_H

#include "chunk.h"
#include "buffer.h"

struct Camera {
    int x;
    int y;
};

class ChunkManager {
    private:
        // All the chunks created
        std::vector<Chunk> chunks;
        // The camera that decides which chunks should be rendered
        Camera camera;

        // The buffer that the world is rendered to
        buffer render_buffer;
    public:
        ChunkManager(int world_width, int world_height);

        // Get the render buffer to display
        buffer get_render_buffer();

        // Write a chunk's data to the render_buffer, 
        // based on the camera offset.
        void render_chunk();

        // Update a chunk's data, using it's update list and max speed to 
        // decide what to update, when.
        void update_chunk(); 
};
#endif
