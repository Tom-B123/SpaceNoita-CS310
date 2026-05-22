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
        std::vector<std::vector<Chunk>> chunks;
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
        ~ChunkManager() {
            delete[] render_buffer.data;
            render_buffer.data = nullptr;
            delete[] swap_requests.data;
            swap_requests.data = nullptr;

            for (auto& row : chunks) {
                for (auto& chunk : row ) {
                    chunk.free();
                }
            }
            chunks.clear();
        }

        // Get the render buffer to display
        buffer get_render_buffer();

        // Write a chunk's data to the render_buffer, 
        // based on the camera offset.
        void render_chunk(CL* cl,Chunk* chunk);

        // Update a chunk's data, using it's update list and max speed to 
        // decide what to update, when.
        void update_chunks(CL* cl); 

        void input(InputState input_state);

        /**
         *  Ensures the data and to_update_count buffers are up to data after CPU side changes
         */
        void refresh_chunk(CL* cl,Chunk* chunk);

        // Go over the border of the chunk and if any swap requests occur, swap the right 2 cells 
        // with the neighbouring chunk
        void process_swap_requests(CL* cl,Chunk* chunk);
        Chunk* get_chunk(int x, int y);

        void free_chunks();
};

#endif
