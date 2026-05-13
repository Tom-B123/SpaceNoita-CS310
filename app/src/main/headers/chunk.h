#ifndef CHUNK_H
#define CHUNK_H

#include "buffer.h"
#include "app.h"
#include "cl_setup.h"

class Chunk {
    private:
        // Stores a buffer with all the pixels that need to be updated
        buffer to_update;
        int to_update_count;
        
    public:
        // Position of the chunk, e.g. 0,0 = 1st chunk, 1,0 = 2nd chunk
        int chunk_x;
        int chunk_y;
        size_t buffer_index;
        int highest_speed = 1;
        Chunk(int x, int y,int size,CL* cl);
        buffer get_data();
        void set_cell(char x, char y, char val);
        void refresh_cells(CL* cl);
};

#endif
