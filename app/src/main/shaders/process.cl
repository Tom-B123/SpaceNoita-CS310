#define MATERIAL_AIR   ' '
#define MATERIAL_SAND  'S'
#define MATERIAL_WATER 'W'
#define MATERIAL_ROCK  'R'


#define PROPERTY_POWDER  (1 << 0)
#define PROPERTY_LIQUID  (1 << 1) 
#define PROPERTY_SOLID   (1 << 2)

__constant uchar material_properties[256] = {
    0,

    [MATERIAL_SAND] = PROPERTY_POWDER,
    [MATERIAL_WATER] = PROPERTY_LIQUID | PROPERTY_POWDER,
    [MATERIAL_ROCK] = PROPERTY_SOLID,
};
__constant uchar material_weights[256] = {
    0,

    [MATERIAL_AIR]   = 128,
    [MATERIAL_SAND]  = 190,
    [MATERIAL_WATER] = 170,
    [MATERIAL_ROCK]  = 255,
};


bool choice_swap(bool do_swap, __global char* data, 
    bool cond, int i1, int i2) {
    char tmp;
    if (do_swap && cond) {
        tmp = data[i1];
        data[i1] = data[i2];
        data[i2] = tmp;
        return true;
    }
    return false;
}

__kernel void process(__global char* data,int iteration, 
        int width, int height, int n_width, int n_height){

    int index = get_global_id(0);

    uint rng = index * 1664525u + iteration * 1103515245u;
    rng = rng * 1103515245u + 12345u;
    float random = (rng & 0x7FFFFFFF) / (float)0x7FFFFFFF;

    bool left_priority = random < 0.5;

    int x = index % ((width + 1)/n_width);
    int y = index / ((width + 1)/n_height);

    x = (x * n_width) + (iteration%n_width);
    y = (y * n_height) + (iteration%n_height);

    int index1 = y * width + x;
    int index2 = y * width + (x+1) % width;
    int index3 = (((y+1)%height) * width) + x;
    int index4 = (((y+1)%height) * width) + (x+1) % width;
    
    int properties1 = material_properties[data[index1]];
    int properties2 = material_properties[data[index2]];
    int properties3 = material_properties[data[index3]];
    int properties4 = material_properties[data[index4]];

    int weight1 = material_weights[data[index1]];
    int weight2 = material_weights[data[index2]];
    int weight3 = material_weights[data[index3]];
    int weight4 = material_weights[data[index4]];

    char tmp;
    bool moved = false;

    // Compare [' ] and [. ]
    if (
        index1 < index3 && 
        properties1 & PROPERTY_POWDER && 
        !(properties3 & PROPERTY_SOLID) && 
        weight1 > weight3
       ){
        tmp = data[index1];
        data[index1] = data[index3];
        data[index3] = tmp;
        moved = true;
    }
    // Compare [ '] and [ .]
    if (
        index2 < index4 && 
        properties2 & PROPERTY_POWDER && 
        !(properties4 & PROPERTY_SOLID) && 
        weight2 > weight4
       ){
        tmp = data[index2];
        data[index2] = data[index4];
        data[index4] = tmp;
        moved = true;
    }
    if (!moved) {
        moved |= choice_swap(
                left_priority,
                data,
                // Compare [' ] and [ .]
                index1 < index4 && 
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
                index2 < index3 && 
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
                index1 < index2 && 
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
                index2 < index1 && 
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
                index3 < index4 && 
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
                index4 < index3 && 
                properties4 & PROPERTY_LIQUID && 
                !(properties3 & PROPERTY_SOLID) && 
                weight4 > weight3,
                index4,
                index3
        );
    }
        /* if (index1 < index2 && properties1 & PROPERTY_LIQUID && !(properties2 & PROPERTY_SOLID) &&  */
        /*         weight1 > weight2) { */
        /*     tmp = data[index1]; */
        /*     data[index1] = data[index2]; */
        /*     data[index2] = tmp; */
        /*     moved = true; */
        /* } */
        /* // Compare [. ] and [ .] */
        /* if (index3 < index4 && properties3 & PROPERTY_LIQUID && !(properties4 & PROPERTY_SOLID) &&  */
        /*         weight3 > weight4) { */
        /*     tmp = data[index3]; */
        /*     data[index3] = data[index4]; */
        /*     data[index4] = tmp; */
        /*     moved = true; */
        /* } */
    /* if (index1 < index3) { */
    /*     if (data[index1] != ' ' && data[index3] == ' ') { */
    /*         char tmp = data[index1]; */
    /*         data[index1] = data[index3]; */
    /*         data[index3] = tmp; */
    /*         moved = true; */
    /*     } */
    /*     if (data[index2] != ' ' && data[index4] == ' ') { */
    /*         char tmp = data[index2]; */
    /*         data[index2] = data[index4]; */
    /*         data[index4] = tmp; */
    /*         moved = true; */
    /*     } */
    /*     if (moved) return; */
    /*     // If index2 has wrapped around, don't make this move */
    /*     if (index1 >= index2) return; */
    /**/
    /**/
    /*     if (is_powder(data[index1]) && data[index1] != ' ' && data[index3] != ' '&& data[index4] == ' ') { */
    /*         char tmp = data[index1]; */
    /*         data[index1] = data[index4]; */
    /*         data[index4] = tmp; */
    /*         moved = true; */
    /*     } */
    /*     if (is_powder(data[index2]) && data[index2] != ' ' && data[index4] != ' '&& data[index3] == ' ') { */
    /*         char tmp = data[index3]; */
    /*         data[index3] = data[index2]; */
    /*         data[index2] = tmp; */
    /*         moved = true; */
    /*     } */
    /*     if (moved) return; */
    /*     if (is_liquid(data[index1]) && data[index1] != ' ' && data[index2] == ' ' || */
    /*         is_liquid(data[index2]) && data[index2] != ' ' && data[index1] == ' ') { */
    /*         char tmp = data[index1]; */
    /*         data[index1] = data[index2]; */
    /*         data[index2] = tmp; */
    /*     } */
    /*     if (is_liquid(data[index3]) && data[index3] != ' ' && data[index4] == ' ' || */
    /*         is_liquid(data[index4]) && data[index4] != ' ' && data[index3] == ' ') { */
    /*         char tmp = data[index3]; */
    /*         data[index3] = data[index4]; */
    /*         data[index4] = tmp; */
    /*     } */
    /* } */
}
