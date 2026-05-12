#ifndef BUFFER_H
#define BUFFER_H

// Number of chars in each buffer_value

struct buffer {
    int width;
    int height;
    char* data;
};

struct buffer_value {
    char material;
};


#define BUFFER_RUN sizeof(buffer_value)

buffer init_buf(int width, int height,char default_val);

void set_buffer(buffer buf, int x, int y,struct buffer_value data);

#endif
