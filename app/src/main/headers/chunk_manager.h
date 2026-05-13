#ifndef CHUNK_MANAGER_H
#define CHUNK_MANAGER_H

#include "chunk.h"
#include "buffer.h"
#include "cl_setup.h"

struct Camera {
    int x;
    int y;
    int speed;
};

struct InputState {
    bool camera_up;
    bool camera_left;
    bool camera_down;
    bool camera_right;
};

class ChunkManager {
    private:
        // All the chunks created
        std::vector<Chunk> chunks;
        // The camera that decides which chunks should be rendered
        Camera camera;

        // The buffer that the world is rendered to
        buffer render_buffer;
        int render_buffer_index;
        
        buffer swap_requests;
        int swap_requests_index;
        // Size of chunks, e.g. 16x16 or 64x64 pixels
        int chunk_size = 64;

        int world_width;
        int world_height;
    public:
        ChunkManager(int world_width, int world_height, CL* cl);

        // Get the render buffer to display
        buffer get_render_buffer();

        // Write a chunk's data to the render_buffer, 
        // based on the camera offset.
        void render_chunk(CL* cl,Chunk chunk);

        // Update a chunk's data, using it's update list and max speed to 
        // decide what to update, when.
        void update_chunks(CL* cl); 

        void input(InputState input_state);

        void refresh_chunk(CL* cl,Chunk chunk);
};

#endif
