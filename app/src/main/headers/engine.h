#ifndef ENGINE_H
#define ENGINE_H
// Handle the game update loop for the player and the world
#include "app.h"
#include "cl_setup.h"

char* init_buf(int width, int height,char default_val);
char* init_output_buf(int width, int height);


void update(CL* cl_components, //cl::Kernel kernel, cl::CommandQueue queue, cl::Buffer mem_buf, 
        int* iteration, int update_count, int width, int height, int n_width, int n_height,
        char* buf, char* output_buf, int data_buffer);

#endif
