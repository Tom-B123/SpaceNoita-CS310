#include "buffer.h"
#include "app.h"
#include <cstring>

buffer init_buf(int width, int height,char default_val) {
    if (width % 2 == 1) width++;
    if (height % 2 == 1) height++;

    char* data = new char[width * height * BUFFER_RUN]();

    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            data[BUFFER_RUN * (i*width + j) + 0] = j; //buffer_value {
            data[BUFFER_RUN * (i*width + j) + 1] = i; //buffer_value {
            data[BUFFER_RUN * (i*width + j) + 2] = default_val; //buffer_value {
        }
    }

    buffer n_buffer = {
        width,
        height,
        data
    };

    return n_buffer;
}

/**
 *  Stores a data struct to the buffer byte array. 
 *  Means that the meaning of each byte is clear and consistent.
 */
void set_buffer(buffer buf, int x, int y,buffer_value data) {
    size_t offset = (y * (buf.width) + x * BUFFER_RUN);

    std::memcpy(buf.data + offset,&data,BUFFER_RUN);
}

/**
 *  Returns a data struct from the buffer byte array. 
 *  Means that the meaning of each byte is clear and consistent.
 */
buffer_value get_buffer(buffer buf, int x, int y) {
    buffer_value v;
    int offset = (y * (buf.width) + x * BUFFER_RUN);

    std::memcpy(&v,buf.data + offset,BUFFER_RUN);
    return v;
}
