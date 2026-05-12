#ifndef BUFFER_H
#define BUFFER_H

// Number of chars in each buffer_value
#define RUN 1;

typedef struct {
    int width;
    int height;
    char* data;
}   buffer;

struct buffer_value {
    char material;
};

buffer init_buf(int width, int height,char default_val);

void set_buffer(int x, int y,struct buffer_value data);

#endif
