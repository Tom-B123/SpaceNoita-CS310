#ifndef CHUNK_H
#define CHUNK_H

#include "buffer.h"
#include "app.h"
#include "cl_setup.h"

class Chunk {
    private:
        // Stores a buffer with all the pixels that need to be updated
        buffer to_update;
        char to_update_count;
        buffer to_update_count_buf;
        // How many times this chunk has been iterated
        int iteration;
        // Handles chunk crossings. Each value is 0 or a buffer_value, with the position in swap_requests 
        // referring to the neighbouring chunk and specific crossing point. 
        // The CPU then swaps the value in this position into the x,y position of the buffer_value
        buffer swap_requests;
    public:
        // Position of the chunk, e.g. 0,0 = 1st chunk, 1,0 = 2nd chunk
        int chunk_x;
        int chunk_y;
        size_t buffer_index;
        size_t to_update_count_index;
        int highest_speed = 1;
        Chunk(int x, int y,int size,CL* cl);
        buffer get_data();
        void set_cell(char x, char y, char val);
        void refresh_cells(CL* cl);
        int get_iteration();
        void iterate();
        void free();
        buffer get_update_count_buf();
};

#endif
