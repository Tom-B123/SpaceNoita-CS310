#include "chunk.h"
#include "buffer.h"

Chunk::Chunk(int x, int y,int size, CL* cl) : chunk_x(x), chunk_y(y) {
    to_update = init_buf(size, size, ' ');
    to_update_count = 0;

    buffer_index = cl->makeBuffer(to_update);
    highest_speed = 1;
}

buffer Chunk::get_data() {
    return to_update;
}

void Chunk::set_cell(char x, char y, char val) {
    set_buffer(to_update, x, y, {x,y,val});
}
