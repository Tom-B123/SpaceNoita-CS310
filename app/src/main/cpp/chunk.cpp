#include "chunk.h"
#include "buffer.h"

Chunk::Chunk(int x, int y,int size, CL* cl) : 
    chunk_x(x), 
    chunk_y(y) 
{
    to_update = init_buf(size, size, ' ');
    // This is an unsigned int value, so stored as 4 bytes.
    //                         1 256 65536  16777216
    char raw_update_count[] = {0,0,  0,     0};
    to_update_count_buf = {4,1,raw_update_count};
    to_update_count_index = cl->makeRenderBuffer(to_update_count_buf);

    buffer_index = cl->makeBuffer(to_update);
    highest_speed = 1;

    iteration = 0;
}

buffer Chunk::get_data() {
    return to_update;
}

void Chunk::set_cell(char x, char y, char val) {
    set_buffer(to_update, x, y, {x,y,val});
}

int Chunk::get_iteration() {
    return iteration;
}
buffer Chunk::get_update_count_buf() {
    return to_update_count_buf;
}

void Chunk::iterate() {
    iteration++;
}

void Chunk::free() {
    delete[] to_update.data;
    std::cout << "Chunk: " << chunk_x << "," << chunk_y << " freed" << std::endl;
}
