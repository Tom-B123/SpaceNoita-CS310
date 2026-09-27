#include "engine.h"
#include "app.h"
// Takes two positions and swaps their data in the databuffer
void swap_cells(int x1, int y1, int x2, int y2,DataPoint* data_buffer) {
    DataPoint tmp = data_buffer[(y1%WORLD_HEIGHT) * WORLD_WIDTH + (x1%WORLD_WIDTH)];
    data_buffer[(y1%WORLD_HEIGHT) * WORLD_WIDTH + (x1%WORLD_WIDTH)] = data_buffer[(y2%WORLD_HEIGHT) * WORLD_WIDTH + (x2%WORLD_WIDTH)];
    data_buffer[(y2%WORLD_HEIGHT) * WORLD_WIDTH + (x2%WORLD_WIDTH)] = tmp;
}

// Sets the material at a given position, replacing air
void spawn_material(int x, int y, char material_id, DataPoint* data_buffer) {
    if (data_buffer[y * WORLD_WIDTH + x].material == ' ') {
        data_buffer[y * WORLD_WIDTH + x].material = material_id;
    }
}

DataPoint* simple_margolus_update(Material* material_data, DataPoint* data_buffer, long step,
                            int x, int y) {

    if (y*2 + step%2 >= WORLD_HEIGHT - 1) { return data_buffer; }
    // Get the data for the top left, top right, bottom left and bottom right cells in the 2x2 grid
    DataPoint tl = data_buffer[((y*2 +(step%2))%WORLD_HEIGHT) * WORLD_WIDTH + (step%2 + (x * 2))%WORLD_WIDTH];
    DataPoint tr = data_buffer[((y*2 +(step%2))%WORLD_HEIGHT) * WORLD_WIDTH + (1 + step%2 + (x * 2))%WORLD_WIDTH];
    DataPoint bl = data_buffer[((y*2 +(1 + step%2))%WORLD_HEIGHT) * WORLD_WIDTH + (step%2 + (x * 2))%WORLD_WIDTH];
    DataPoint br = data_buffer[((y*2 +(1 + step%2))%WORLD_HEIGHT) * WORLD_WIDTH + (1 + step%2 + (x * 2))%WORLD_WIDTH];

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
        if ((tl_state == POWDER || tl_state == LIQUID) && bl_state != SOLID) {
            swap_cells(x * 2 + step%2, y*2 + step%2, x*2 + step % 2, 1 + y * 2 + step % 2,data_buffer);
            swapped = true;
        }
    }
    if (tr_density > br_density) {
        if ((tr_state == POWDER || tr_state == LIQUID) && br_state != SOLID) {
            swap_cells(1 + x * 2 + step%2, y*2 + step%2,1 + x*2 + step % 2, 1 + y * 2 + step % 2,data_buffer);
            swapped = true;
        }
    }
    if (!swapped && tl_density > br_density) {
        if ((tl_state == POWDER || tl_state == LIQUID) && br_state != SOLID) {
            swap_cells(x * 2 + step%2, y*2 + step%2,1 + x*2 + step % 2, 1 + y * 2 + step % 2,data_buffer);
            swapped = true;
        }
    }
    if (!swapped && tr_density > bl_density) {
        if ((tr_state == POWDER || tr_state == LIQUID) && bl_state != SOLID) {
            swap_cells(1 + x * 2 + step%2, y*2 + step%2,x*2 + step % 2, 1 + y * 2 + step % 2,data_buffer);
            swapped = true;
        }
    }
    if (!swapped && tl_density > tr_density) {
        if ((tl_state == LIQUID) && tr_state != SOLID) {
            swap_cells(x * 2 + step%2, y*2 + step%2,1 + x*2 + step % 2,y * 2 + step % 2,data_buffer);
            swapped = true;
        }
    }
    if (!swapped && bl_density > br_density) {
        if ((bl_state == LIQUID) && br_state != SOLID) {
            swap_cells(x * 2 + step%2,1 + y*2 + step%2,1 + x*2 + step % 2, 1 + y * 2 + step % 2,data_buffer);
            swapped = true;
        }
    }
    if (!swapped && tr_density > tl_density) {
        if ((tr_state == LIQUID) && tl_state != SOLID) {
            swap_cells(1 + x * 2 + step%2, y*2 + step%2,x*2 + step % 2,y * 2 + step % 2,data_buffer);
            swapped = true;
        }
    }
    if (!swapped && br_density > bl_density) {
        if ((br_state == LIQUID) && bl_state != SOLID) {
            swap_cells(1 + x * 2 + step%2,1 + y*2 + step%2,x*2 + step % 2, 1 + y * 2 + step % 2,data_buffer);
            swapped = true;
        }
    }

    return data_buffer;
}
