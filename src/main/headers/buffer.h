#ifndef BUFFER_H
#define BUFFER_H

// Number of chars in each buffer_value

struct buffer {
    int width;
    int height;
    char* data;
};

struct buffer_value {
    char x;
    char y;
    char material;
    char xvel;
    char yvel;
    char flags;
};


#define BUFFER_RUN sizeof(buffer_value)

buffer init_buf(int width, int height,char default_val);
buffer init_render_buf(int width, int height,char default_val);

void set_buffer(buffer buf, int x, int y,buffer_value data);
void set_buffer(buffer buf, size_t index,buffer_value data);
buffer_value get_buffer(buffer buf, int x, int y);



void free_buffer(buffer buf);

#endif
