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
}   buffer_value;

#define BUFFER_RUN sizeof(buffer_value)

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


bool choice_swap(bool do_swap, __global char* data, 
    bool cond, int i1, int i2) {
    buffer_value tmp;
    if (do_swap && cond) {
        tmp = get_buffer(data,i1);
        set_buffer(data,i1,get_buffer(data,i2));
        set_buffer(data,i2,tmp);
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

__kernel void render(__global char* data, __global char* render_buffer) {
    int index = get_global_id(0);

    buffer_value val = get_buffer(data,index);

    render_buffer[index] = val.material;
}

__kernel void process(__global char* data,int iteration, 
        int width, int height, int n_width, int n_height, 
        __global char* spawners){

    int index = get_global_id(0);

    // X -> index wrapped around width
    int x = index % (width/n_width);
    // Y -> index divided by width
    int y = index / (width/n_height);

    /* if (index == 0) { printf("Val: %u\n",randint(iteration,x,y)); } */
    
    bool left_priority = randint(iteration,x,y) % 2 == 0;

    /* left_priority = left_priority && iteration%3 == 0; */

    x = (x * n_width) + (iteration%n_width);
    y = (y * n_height) + (iteration%n_height);

    int rotation_step = (iteration) % 3600;
    int rotation_direction = ((iteration) / 3600) % 4;

    int direction = (rotation_direction + (randint(iteration,x,y) % 3600 < rotation_step)) % 4;

    direction = 0;

    int indicies[] = {
        y * width + x,                          // [' ]
        y * width + (x+1) % width,              // [ ']
        (((y+1)%height) * width) + x,           // [. ]
        (((y+1)%height) * width) + (x+1) % width// [ .]
    };
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

    if (direction == 3 || direction == 1) { 
        int tmp = x; x = y; y = tmp;
        tmp = width; width = height; height = tmp;
    }

    /* if (spawners[index1] > 0) { */
    /*     if (data[index1] == MATERIAL_AIR) { */
    /*         data[index1] = spawners[index1]; */
    /*         return; */
    /*     } */
    /* } */
    /* if (spawners[index2] > 0) { */
    /*     if (data[index2] == MATERIAL_AIR) { */
    /*         data[index2] = spawners[index2]; */
    /*         return; */
    /*     } */
    /* } */
    /* if (spawners[index3] > 0) { */
    /*     if (data[index3] == MATERIAL_AIR) { */
    /*         data[index3] = spawners[index3]; */
    /*         return; */
    /*     } */
    /* } */
    /* if (spawners[index4] > 0) { */
    /*     if (data[index4] == MATERIAL_AIR) { */
    /*         data[index4] = spawners[index4]; */
    /*         return; */
    /*     } */
    /* } */

    int properties1 = material_properties[get_buffer(data,index1).material];
    int properties2 = material_properties[get_buffer(data,index2).material];
    int properties3 = material_properties[get_buffer(data,index3).material];
    int properties4 = material_properties[get_buffer(data,index4).material];

    int weight1 = material_weights[get_buffer(data,index1).material];
    int weight2 = material_weights[get_buffer(data,index2).material];
    int weight3 = material_weights[get_buffer(data,index3).material];
    int weight4 = material_weights[get_buffer(data,index4).material];

    char tmp;
    bool moved = false;

    // Compare [' ] and [. ]
    moved |=choice_swap(
            true,
            data,
            // Compare [. ] and [ ']
            y < height - 1 && y >= 0 &&
            properties1 & PROPERTY_POWDER && 
            !(properties3 & PROPERTY_SOLID) && 
            weight1 > weight3,
            index1,
            index3
    );
    /* if ( */
    /*     y < height - 1 && y >= 0 && */
    /*     properties1 & PROPERTY_POWDER &&  */
    /*     !(properties3 & PROPERTY_SOLID) &&  */
    /*     weight1 > weight3 */
    /*    ){ */
    /*     tmp = data[index1]; */
    /*     data[index1] = data[index3]; */
    /*     data[index3] = tmp; */
    /*      set_buffer(data,index1,get_buffer(data,index3)); */ 
    /*      set_buffer(data,index3, tmp); */ 
    /*     moved = true; */
    /* } */
    // Compare [ '] and [ .]
    moved |=choice_swap(
            true,
            data,
            // Compare [. ] and [ ']
            y < height - 1 && y >= 0 &&
            properties2 & PROPERTY_POWDER && 
            !(properties4 & PROPERTY_SOLID) && 
            weight2 > weight4,
            index2,
            index4
    );
    /* if ( */
    /*     y  < height - 1 && y >= 0 && */
    /*     properties2 & PROPERTY_POWDER &&  */
    /*     !(properties4 & PROPERTY_SOLID) &&  */
    /*     weight2 > weight4 */
    /*    ){ */
    /*     tmp = data[index2]; */
    /*     data[index2] = data[index4]; */
    /*     data[index4] = tmp; */
    /*     moved = true; */
    /* } */
    if (!moved) {
        moved |= choice_swap(
                left_priority,
                data,
                // Compare [' ] and [ .]
                x < width-1 && 
                y  < height - 1 && y >= 0 &&
                properties1 & PROPERTY_POWDER && 
                !(properties4 & PROPERTY_SOLID) && 
                weight1 > weight4,
                index1,
                index4
        );
    }
    if (!moved) {
        moved |=choice_swap(
                !left_priority,
                data,
                // Compare [. ] and [ ']
                x < width - 1 && 
                y < height - 1 && y >= 0 &&
                properties2 & PROPERTY_POWDER && 
                !(properties3 & PROPERTY_SOLID) && 
                weight2 > weight3,
                index2,
                index3
        );
    }
    if (!moved) {
        // Compare [' ] and [ ']
        moved |=choice_swap(
                left_priority,
                data,
                // Compare [' ] and [ ']
                x < width - 1 && 
                y < height - 1 && y >= 0 &&
                properties1 & PROPERTY_LIQUID && 
                !(properties2 & PROPERTY_SOLID) && 
                weight1 > weight2,
                index1,
                index2
        );
    }
    if (!moved) {
        moved |= choice_swap(
                !left_priority,
                data,
                // Compare [ '] and [' ]
                x < width - 1 && 
                y < height - 1 && y >= 0 &&
                properties2 & PROPERTY_LIQUID && 
                !(properties1 & PROPERTY_SOLID) && 
                weight2 > weight1,
                index2,
                index1
        );
    }
    if (!moved) {
        moved |=choice_swap(
                left_priority,
                data,
                // Compare [. ] and [ .]
                x < width - 1 && 
                y < height - 1 && 
                properties3 & PROPERTY_LIQUID && 
                !(properties4 & PROPERTY_SOLID) && 
                weight3 > weight4,
                index3,
                index4
        );
    }
    if (!moved) {
        moved |=choice_swap(
                !left_priority,
                data,
                // Compare [ .] and [. ]
                x < width - 1 && 
                y >= 0 && y < height - 1 && 
                properties4 & PROPERTY_LIQUID && 
                !(properties3 & PROPERTY_SOLID) && 
                weight4 > weight3,
                index4,
                index3
        );
    }
}
