#ifndef ENGINE_H
#define ENGINE_H
// Handle the game update loop for the player and the world
#include "app.h"
#include "cl_setup.h"


void update(CL* cl_components, //cl::Kernel kernel, cl::CommandQueue queue, cl::Buffer mem_buf, 
        int* iteration, int update_count, int n_width, int n_height,
        buffer buf, int data_buffer);

#endif
