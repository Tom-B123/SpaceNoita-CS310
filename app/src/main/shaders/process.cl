#define MATERIAL_AIR   ' '
#define MATERIAL_SAND  'S'
#define MATERIAL_WATER 'W'
#define MATERIAL_ROCK  'R'
#define MATERIAL_OIL   'O'


#define PROPERTY_POWDER  (1 << 0)
#define PROPERTY_LIQUID  (1 << 1) 
#define PROPERTY_SOLID   (1 << 2)


typedef struct {
    char x;
    char y;
    char material;
    char xvel;
    char yvel;
    char flags;
}   buffer_value;

#define BUFFER_RUN sizeof(buffer_value)

void increase_active_count(volatile __global unsigned int* count) {
    atomic_inc(count);
}

void set_buffer(__global char* data, int index, buffer_value value) {
    size_t offset = (index * BUFFER_RUN);

    ((__global buffer_value* )data)[index] = value;
}



buffer_value get_buffer(__global char* data, int index) {
    int offset = (index * BUFFER_RUN);

    __global buffer_value* v = ((__global buffer_value*)(data + offset));
    return *v;
}

__constant uchar material_properties[256] = {
    0,

    [MATERIAL_SAND] = PROPERTY_POWDER,
    [MATERIAL_WATER]= PROPERTY_LIQUID | PROPERTY_POWDER,
    [MATERIAL_OIL]  = PROPERTY_LIQUID | PROPERTY_POWDER,
    [MATERIAL_ROCK] = PROPERTY_SOLID,
};
__constant uchar material_weights[256] = {
    0,

    [MATERIAL_AIR]   = 128,
    [MATERIAL_SAND]  = 190,
    [MATERIAL_WATER] = 170,
    [MATERIAL_OIL] = 150,
    [MATERIAL_ROCK]  = 255,
};


/* void request_swap(buffer_value bv1, buffer_value bv2, int chunk_size, __global char* swap_requests) { */
/*     int side = -1; */
/*     buffer_value swap_value = {0,0,0}; */
/*     if (bv1.material != MATERIAL_AIR && bv1.x >= chunk_size-1) {  */
/*         swap_value = bv1;  */
/*         side = 1;  */
/*     } */
/*     else if (bv2.material != MATERIAL_AIR && bv2.x >= chunk_size-1) {  */
/*         swap_value = bv2;  */
/*         side = 1;  */
/*     } */
/*     else if (bv1.material != MATERIAL_AIR && bv1.y >= chunk_size-1) {  */
/*         swap_value = bv1;  */
/*         side = 2;  */
/*     } */
/*     else if (bv2.material != MATERIAL_AIR && bv2.y >= chunk_size-1){  */
/*         swap_value = bv2;  */
/*         side = 2;  */
/*     } */
/**/
/*     if (side > -1) { */
/*         printf("Value: [%c] exitied on side: %i\n",swap_value.material,side); */
/*     } */
/* } */

bool choice_swap(bool do_swap, __global char* to_update, bool valid_x, bool valid_y,
    bool in_bounds, bool cond, int i1, int i2, int chunk_size) {
    
    if (!in_bounds) {

        // Delete both out of bounds cells, might lead to the material on the opposite side of the chunk getting deleted?
        buffer_value bv1 = get_buffer(to_update,i1);

        int side = -1;
        /* buffer_value bv2 = get_buffer(to_update,i2); */
        /* set_buffer(to_update,i2,bv2); */
        
        if (material_properties[bv1.material] & PROPERTY_POWDER) {

            if (!valid_x){
                if (bv1.x > chunk_size/2) {
                    side = 1;
                }
                else {
                    side = 3;
                }
            }
            if (!valid_y) {
                if (bv1.y > chunk_size/2) {
                    side = 2;
                }
                else {
                    side = 0;
                }
            }
            if ((!valid_x || !valid_y) && side > -1) {
                /* switch(side) { */
                /*     case 0: printf("North exit!\n"); break; */
                /*     case 1: printf("East exit!\n"); break; */
                /*     case 2: printf("South exit!\n"); break; */
                /*     case 3: printf("West exit!\n"); break; */
                /* } */
                if (side == 0 || side == 2) {
                }   else {
                }
                /* bv1.material = MATERIAL_AIR; */
            }
        }

        /* set_buffer(to_update,i1,bv1); */
        return false;
    }
    else if (do_swap && cond) {
        char tmp;
        buffer_value val1 = get_buffer(to_update,i1);
        buffer_value val2 = get_buffer(to_update,i2);
        
        tmp = val1.material;
        val1.material = val2.material;
        val2.material = tmp;

        set_buffer(to_update,i1,val1);
        set_buffer(to_update,i2,val2);
        return true;
    }
    return false;
}

uint randint(int iteration, int x, int y) {
    uint bit = 0;
    for (int i = 0; i < 32; i++) {
        bit = bit << 1;
        bit |= ((iteration << 3) % 13)+1 >= (iteration + ((x * 31 + i + y * 37) << 5)) % 17;
    }
    return bit;
}

__kernel void render(__global char* data, 
    int chunk_x, int chunk_y, int chunk_size,
    int camera_x, int camera_y,
    int width, int height,
    __global char* render_buffer) 
{
    int index = get_global_id(0);

    /* if (index == 0) printf("kernel width: %i\n",width); */

    buffer_value val = get_buffer(data,index);

    int x = val.x;//index % chunk_size;
    int y = val.y;//index / chunk_size;

    int rx = x + camera_x + chunk_x * chunk_size;
    int ry = y + camera_y + chunk_y * chunk_size;

    if (rx < 0 || rx >= width || ry < 0 || ry >= height) return;
    render_buffer[ry * width + rx] = val.material;
}

__kernel void process(__global char* to_update,int iteration, 
        int chunk_x, int chunk_y, int chunk_size,
        int width, int height, int n_width, int n_height, 
        __global unsigned int* to_update_count) {

    int index = get_global_id(0);

    increase_active_count(to_update_count);
    if (index == 0) {
        printf("Update count: %u\n",*to_update_count);
    }

    buffer_value val = get_buffer(to_update,index);

    int x = index % (chunk_size/n_width);
    // Y -> index divided by width
    int y = index / (chunk_size/n_height);

    bool left_priority = randint(iteration,x + chunk_x * chunk_size,y + chunk_y * chunk_size) % 64 > 31;

    x = (x * n_width) + (iteration%n_width);
    y = (y * n_height) + (iteration%n_height);


    int ox = chunk_x * chunk_size;
    int oy = chunk_y * chunk_size * width;

    int offset = 0;//ox+oy;

    int rotation_step = (iteration) % 3600;
    int rotation_direction = ((iteration) / 3600) % 4;

    int direction = (rotation_direction + (randint(iteration,x,y) % 3600 < rotation_step)) % 4;

    direction = 0;

    int indicies[] = {
        y * (chunk_size) + x,                                      // [' ]
        y * (chunk_size) + (x+1) % chunk_size,                     // [ ']
        (((y+1)%chunk_size) * (chunk_size)) + x,                   // [. ]
        (((y+1)%chunk_size) * (chunk_size)) + (x+1) % chunk_size   // [ .]
    };

    bool valid_x = x < chunk_size - 1;
    bool valid_y = y < chunk_size - 1;
    int orderings[] = {
        //N:
        0,1,2,3,
        //E:
        1,3,0,2,
        //S:
        3,2,1,0,
        //W:
        2,0,3,1,
    };

    int index1 = indicies[orderings[4 * direction + 0]];
    int index2 = indicies[orderings[4 * direction + 1]];
    int index3 = indicies[orderings[4 * direction + 2]];
    int index4 = indicies[orderings[4 * direction + 3]];

    int test = 0;

    if (direction == 3 || direction == 1) { 
        int tmp = x; x = y; y = tmp;
        tmp = width; width = height; height = tmp;
    }

    int properties1 = material_properties[get_buffer(to_update,index1).material];
    int properties2 = material_properties[get_buffer(to_update,index2).material];
    int properties3 = material_properties[get_buffer(to_update,index3).material];
    int properties4 = material_properties[get_buffer(to_update,index4).material];

    int weight1 = material_weights[get_buffer(to_update,index1).material];
    int weight2 = material_weights[get_buffer(to_update,index2).material];
    int weight3 = material_weights[get_buffer(to_update,index3).material];
    int weight4 = material_weights[get_buffer(to_update,index4).material];

    char tmp;
    bool moved = false;

    /* if (index == 0) printf("Iteration: %i\n",iteration); */

    // Compare [' ] and [. ]
    moved |=choice_swap(
            true,
            to_update,
            valid_x,valid_y,
            // Compare [. ] and [ ']
            y < chunk_size - 1 && y >= 0,
            properties1 & PROPERTY_POWDER && 
            !(properties3 & PROPERTY_SOLID) && 
            weight1 > weight3,
            index1,
            index3,
            chunk_size
    );
    // Compare [ '] and [ .]
    moved |=choice_swap(
            true,
            to_update,
            valid_x,valid_y,
            // Compare [. ] and [ ']
            y < chunk_size - 1 && y >= 0,
            properties2 & PROPERTY_POWDER && 
            !(properties4 & PROPERTY_SOLID) && 
            weight2 > weight4,
            index2,
            index4,
            chunk_size
    );
    if (!moved) {
        moved |= choice_swap(
                left_priority,
                to_update,
                valid_x,valid_y,
                // Compare [' ] and [ .]
                x < chunk_size-1 && 
                y  < chunk_size - 1 && y >= 0,
                properties1 & PROPERTY_POWDER && 
                !(properties4 & PROPERTY_SOLID) && 
                weight1 > weight4,
                index1,
                index4,
                chunk_size
        );
    }
    if (!moved) {
        moved |=choice_swap(
                !left_priority,
                to_update,
                valid_x,valid_y,
                // Compare [. ] and [ ']
                x < chunk_size - 1 && 
                y < chunk_size - 1 && y >= 0,
                properties2 & PROPERTY_POWDER && 
                !(properties3 & PROPERTY_SOLID) && 
                weight2 > weight3,
                index2,
                index3,
                chunk_size
        );
    }
    if (!moved) {
        // Compare [' ] and [ ']
        moved |=choice_swap(
                left_priority,
                to_update,
                valid_x,valid_y,
                // Compare [' ] and [ ']
                x < chunk_size - 1 && 
                y < chunk_size - 1 && y >= 0,
                properties1 & PROPERTY_LIQUID && 
                !(properties2 & PROPERTY_SOLID) && 
                weight1 > weight2,
                index1,
                index2,
                chunk_size
        );
    }
    if (!moved) {
        moved |= choice_swap(
                !left_priority,
                to_update,
                valid_x,valid_y,
                // Compare [ '] and [' ]
                x < chunk_size - 1 && 
                y < chunk_size - 1 && y >= 0,
                properties2 & PROPERTY_LIQUID && 
                !(properties1 & PROPERTY_SOLID) && 
                weight2 > weight1,
                index2,
                index1,
                chunk_size
        );
    }
    if (!moved) {
        moved |=choice_swap(
                left_priority,
                to_update,
                valid_x,valid_y,
                // Compare [. ] and [ .]
                x < chunk_size - 1 && 
                y < chunk_size - 1 ,
                properties3 & PROPERTY_LIQUID && 
                !(properties4 & PROPERTY_SOLID) && 
                weight3 > weight4,
                index3,
                index4,
                chunk_size
        );
    }
    if (!moved) {
        moved |=choice_swap(
                !left_priority,
                to_update,
                valid_x,valid_y,
                // Compare [ .] and [. ]
                x < chunk_size - 1 && 
                y >= 0 && y < chunk_size - 1,
                properties4 & PROPERTY_LIQUID && 
                !(properties3 & PROPERTY_SOLID) && 
                weight4 > weight3,
                index4,
                index3,
                chunk_size
        );
    }
}
