#include "engine.h"
#include "app.h"
#include "materials.h"
// Takes two positions and swaps their data in the databuffer
void swap_cells(int x1, int y1, int x2, int y2,DataPoint* data_buffer) {
    DataPoint tmp = data_buffer[(y1%WORLD_HEIGHT) * WORLD_WIDTH + (x1%WORLD_WIDTH)];
    data_buffer[(y1%WORLD_HEIGHT) * WORLD_WIDTH + (x1%WORLD_WIDTH)] = data_buffer[(y2%WORLD_HEIGHT) * WORLD_WIDTH + (x2%WORLD_WIDTH)];
    data_buffer[(y2%WORLD_HEIGHT) * WORLD_WIDTH + (x2%WORLD_WIDTH)] = tmp;
}
void swap_cells(int indexA,int indexB,DataPoint* data_buffer) {
    DataPoint tmp = data_buffer[indexA];
    data_buffer[indexA] = data_buffer[indexB];
    data_buffer[indexB] = tmp;
}

// Sets the material at a given position, repintacing air
void spawn_material(int x, int y, char material_id, DataPoint* data_buffer) {
    if (data_buffer[y * WORLD_WIDTH + x].material == ' ') {
        data_buffer[y * WORLD_WIDTH + x].material = material_id;
    }
}

DataPoint* simple_margolus_update(Material* material_data, DataPoint* data_buffer, long step,
                            int x, int y) {

    if (y*2 + step%2 >= WORLD_HEIGHT - 1) { return data_buffer; }

    int chunk_size = WORLD_WIDTH;
    // Which direction is "up"? 0 -> North, 1 -> East etc.
    int direction = step / 600;

    direction = direction % 4;
    
    // if (direction == 3 || direction == 1) { 
    //     int tmp = x; x = y; y = tmp;
    //     tmp = WORLD_WIDTH; width = height; height = tmp;
    // }

    int nx = 2*x + (step%2);
    int ny = 2*y + (step%2);
    int indicies[] = {
        ny * (chunk_size) + nx,                                      // [' ]
        ny * (chunk_size) + (nx+1) % chunk_size,                     // [ ']
        (((ny+1)%chunk_size) * (chunk_size)) + nx,                   // [. ]
        (((ny+1)%chunk_size) * (chunk_size)) + (nx+1) % chunk_size   // [ .]
    };

    bool valid_x = nx < chunk_size - 1;
    bool valid_y = ny < chunk_size - 1;
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

    // Get the data for the top left, top right, bottom left and bottom right cells in the 2x2 grid
    // DataPoint tl = data_buffer[((y*2 +(step%2))%WORLD_HEIGHT) * WORLD_WIDTH + (step%2 + (x * 2))%WORLD_WIDTH];
    // DataPoint tr = data_buffer[((y*2 +(step%2))%WORLD_HEIGHT) * WORLD_WIDTH + (1 + step%2 + (x * 2))%WORLD_WIDTH];
    // DataPoint bl = data_buffer[((y*2 +(1 + step%2))%WORLD_HEIGHT) * WORLD_WIDTH + (step%2 + (x * 2))%WORLD_WIDTH];
    // DataPoint br = data_buffer[((y*2 +(1 + step%2))%WORLD_HEIGHT) * WORLD_WIDTH + (1 + step%2 + (x * 2))%WORLD_WIDTH];
    
    DataPoint tl = data_buffer[index1];
    DataPoint tr = data_buffer[index2];
    DataPoint bl = data_buffer[index3];
    DataPoint br = data_buffer[index4];


    // Get the corresponding densities
    int tl_density = material_data[tl.material].density;
    int tr_density = material_data[tr.material].density;
    int bl_density = material_data[bl.material].density;
    int br_density = material_data[br.material].density;

    int tl_state = material_data[tl.material].state;
    int tr_state = material_data[tr.material].state;
    int bl_state = material_data[bl.material].state;
    int br_state = material_data[br.material].state;
    bool swapped = false;
    // Defines the rules for when to swap cells
    if (tl_density > bl_density) {
        if ((tl_state == POWDER || tl_state == LIQUID || tl_state == GAS) && bl_state != SOLID) {
            swap_cells(index1,index3,data_buffer);
            swapped = true;
        }
    }
    if (tr_density > br_density) {
        if ((tr_state == POWDER || tr_state == LIQUID || tr_state == GAS) && br_state != SOLID) {
            swap_cells(index2,index4,data_buffer);
            swapped = true;
        }
    }
    if (!swapped && tl_density > br_density) {
        if ((tl_state == POWDER || tl_state == LIQUID || tl_state == GAS) && br_state != SOLID) {
            swap_cells(index1,index4,data_buffer);
            swapped = true;
        }
    }
    if (!swapped && tr_density > bl_density) {
        if ((tr_state == POWDER || tr_state == LIQUID || tr_state == GAS) && bl_state != SOLID) {
            swap_cells(index2,index3,data_buffer);
            swapped = true;
        }
    }
    if (!swapped && tl_density > tr_density) {
        if ((tl_state == LIQUID || tl_state == GAS) && tr_state != SOLID) {
            swap_cells(index1,index2,data_buffer);
            swapped = true;
        }
    }
    if (!swapped && bl_density > br_density) {
        if ((bl_state == LIQUID || bl_state == GAS) && br_state != SOLID) {
            swap_cells(index3,index4,data_buffer);
            swapped = true;
        }
    }
    if (!swapped && tr_density > tl_density) {
        if ((tr_state == LIQUID || tr_state == GAS) && tl_state != SOLID) {
            swap_cells(index2,index1,data_buffer);
            swapped = true;
        }
    }
    if (!swapped && br_density > bl_density) {
        if ((br_state == LIQUID || br_state == GAS) && bl_state != SOLID) {
            swap_cells(index4,index3,data_buffer);
            swapped = true;
        }
    }

    return data_buffer;
}
