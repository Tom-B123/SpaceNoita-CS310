#include "buffer.h"
#include "app.h"

buffer init_buf(int width, int height,char default_val) {
    if (width % 2 == 1) width++;
    if (height % 2 == 1) height++;

    char* data = new char[width * height]();

    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            data[i*width + j] = default_val;
        }
    }

    buffer n_buffer = {
        width,
        height,
        data
    };

    return n_buffer;
}

void set_buffer(buffer buf, int x, int y,buffer_value data) {
    size_t offset = (y * (buf.width) + x * BUFFER_RUN);

    std::memcpy(buf.data + offset,&data,BUFFER_RUN);
}
