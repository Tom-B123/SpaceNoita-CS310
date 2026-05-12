#ifndef CHUNK_H
#define CHUNK_H

#include "buffer.h"
#include "app.h"

class Chunk {
    private:
        // Stores a buffer with all the pixels that need to be updated
        buffer to_update;
        int to_update_count;
        // Position of the chunk, e.g. 0,0 = 1st chunk, 1,0 = 2nd chunk
        int chunk_x;
        int chunk_y;


        // Size of chunks, e.g. 16x16 or 64x64 pixels
        int size = 64;
    public:
        Chunk() {

        }
};

#endif
