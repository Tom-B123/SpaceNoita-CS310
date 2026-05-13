#include "chunk.h"
#include "buffer.h"

Chunk::Chunk(int x, int y,int size) : chunk_x(x), chunk_y(y) {
    to_update = init_buf(size, size, 'S');
    to_update_count = 0;
}

buffer Chunk::get_data() {
    return to_update;
}
