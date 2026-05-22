#include "chunk.h"
#include "buffer.h"

Chunk::Chunk(int x, int y,int size, CL* cl) : 
    chunk_x(x), 
    chunk_y(y) 
{
    to_update = init_buf(size, size, ' ');
    // This is an unsigned int value, so stored as 4 bytes.
    //                         1 256 65536  16777216
    // char raw_update_count[] = {0,0,  0,     0};
    to_update_count_buf = {4,1,new char[]{0,0,0,0}};
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

int Chunk::get_update_count() {
    unsigned int out = 0;
    for (int i = 3; i >= 0; i--) {
        out <<= 8;
        out += (unsigned char)(to_update_count_buf.data[i]);
    }
    return out;
}

void Chunk::increment_update_count() {
    unsigned int update_count = get_update_count();
    update_count++;
    

    for (int i = 0; i < 4; i++) {
        to_update_count_buf.data[i] = update_count & 0xFF;
        update_count >>= 8;
    }

}
void Chunk::decrement_update_count() {
    unsigned int update_count = get_update_count();
    update_count--;
    

    for (int i = 0; i < 4; i++) {
        to_update_count_buf.data[i] = update_count & 0xFF;
        update_count >>= 8;
    }

}
void Chunk::iterate() {
    iteration++;
}

void Chunk::free() {
    delete[] to_update.data;
    std::cout << "Chunk: " << chunk_x << "," << chunk_y << " freed" << std::endl;
}
