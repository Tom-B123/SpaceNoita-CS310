#include "buffer.h"
#include "app.h"
#include <cstring>

buffer init_buf(int width, int height,char default_val) {
    char* data = new char[width * height * BUFFER_RUN]();

    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            data[BUFFER_RUN * (i*width + j) + 0] = j;             
            data[BUFFER_RUN * (i*width + j) + 1] = i;
            data[BUFFER_RUN * (i*width + j) + 2] = default_val;
            data[BUFFER_RUN * (i*width + j) + 3] = 0;
            data[BUFFER_RUN * (i*width + j) + 4] = 0;
            data[BUFFER_RUN * (i*width + j) + 5] = 0;
        }
    }

    buffer n_buffer = {
        width,
        height,
        data
    };

    return n_buffer;
}
buffer init_render_buf(int width, int height,char default_val) {
    char* data = new char[width * height]();

    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            data[i*width + j] = default_val; //buffer_value {
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
    size_t index = y * (buf.width) + x;
    set_buffer(buf,index,data);
}
void set_buffer(buffer buf, size_t index,buffer_value data) {
    std::memcpy(buf.data + index * BUFFER_RUN,&data,BUFFER_RUN);
}

/**
 *  Returns a data struct from the buffer byte array. 
 *  Means that the meaning of each byte is clear and consistent.
 */
buffer_value get_buffer(buffer buf, int x, int y) {
    buffer_value v;
    int offset = BUFFER_RUN * (y * (buf.width) + x);

    std::memcpy(&v,buf.data + offset,BUFFER_RUN);
    return v;
}
