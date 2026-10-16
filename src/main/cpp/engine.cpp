#include "engine.h"
#include "app.h"
#include "materials.h"
#include <algorithm>
#include <vector>
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

// Return the index of the largest of four floats, using 3 conditionals
int largest_of_four(float A, float B, float C, float D) {
    if (A > B) {
        if (A > C) {
            if (A > D) {
                // A is largest
                return 0;
            }   else {
                // D is largest
                return 3;
            }
        }   else {
            if (C > D) {
                // C is largest
                return 2;
            }   else {
                // D is largest
                return 3;
            }
        }
        // B > A case
    }   else {
        if (B > C) {
            if (B > D) {
                // A is largest
                return 1;
            }   else {
                // D is largest
                return 3;
            }
        }   else {
            if (C > D) {
                // C is largest
                return 2;
            }   else {
                // D is largest
                return 3;
            }
        }
    }
}

int next_direction(DataPoint* data) {
    // based on Bresenhams (from wikipedia)
    
    // Subtract the currently moved steps from the remaining velocity.
    float x_vel = data->x_vel - data->frame_x_vel;
    float y_vel = data->y_vel - data->frame_y_vel;

    // Decide which direction is better
    
    // Return this direction and update the value of frame_vel
    return 0;
}

/**
 *  Computes the next value for the data_buffer, at the given cell position.
 *  Mimics the GPU code for a single thread
 *  @param 
 */
DataPoint* simple_margolus_update(Material* material_data, DataPoint* data_buffer, long step,
                            int x, int y) {

    if (y*2 + step%2 >= WORLD_HEIGHT - 1) { return data_buffer; }

    int chunk_size = WORLD_WIDTH;

    // Get the normalised x,y position for the top left corner
    int nx = 2*x + (step%2);
    int ny = 2*y + (step%2);
    int indicies[] = {
        ny * (chunk_size) + nx,                                      // [' ]
        ny * (chunk_size) + (nx+1) % chunk_size,                     // [ ']
        (((ny+1)%chunk_size) * (chunk_size)) + nx,                   // [. ]
        (((ny+1)%chunk_size) * (chunk_size)) + (nx+1) % chunk_size   // [ .]
    };

    // The order in which top left, top right, bottom left and bottom right cells
    // are interpretted to "trick" the simulation into giving the right logic.
    // 0,1,2,3 -> 1,3,0,2 rotates the rules to work horizontally.
    // 2 more rules for each reflection
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

    // Which direction is "up"? 0 -> North, 1 -> East etc.
    // We assume direction is 0, then calculate what it should be for this cell
    int direction = 0;

    int index1 = indicies[orderings[4 * direction + 0]];
    int index2 = indicies[orderings[4 * direction + 1]];
    int index3 = indicies[orderings[4 * direction + 2]];
    int index4 = indicies[orderings[4 * direction + 3]];

    // Get the data from the cell, oriented with the top cells pointing towards [direction] 
    // and the bottom cells pointing away.
    DataPoint tl = data_buffer[index1];
    DataPoint tr = data_buffer[index2];
    DataPoint bl = data_buffer[index3];
    DataPoint br = data_buffer[index4];

    int tl_density = material_data[tl.material].density;
    int tr_density = material_data[tr.material].density;
    int bl_density = material_data[bl.material].density;
    int br_density = material_data[br.material].density;

    // Calculate the momentum for each cell (sqaured length of velocity * density)
    // The highest momentum determines which direction this cell should update in.
    // Should account for the frame distance travelled
    float momentum_tl = tl_density * tl_density * tl.x_vel * tl.x_vel + tl_density * tl_density * tl.y_vel * tl.y_vel;
    float momentum_tr = tr_density * tr_density * tr.x_vel * tr.x_vel + tr_density * tr_density * tr.y_vel * tr.y_vel;
    float momentum_bl = bl_density * bl_density * bl.x_vel * bl.x_vel + bl_density * bl_density * bl.y_vel * bl.y_vel;
    float momentum_br = br_density * br_density * br.x_vel * br.x_vel + br_density * br_density * br.y_vel * br.y_vel;

    // Get the highest momentum and get the right direction from this.
    int dominant = largest_of_four(momentum_tl, momentum_tr, momentum_bl, momentum_br);
    
    switch (dominant) {
        case 0: // TL
            direction = next_direction(&tl);
            break;
        case 1: // TR
            direction = next_direction(&tr);
            break;
        case 2: // BL
            direction = next_direction(&bl);
            break;
        case 3: // BR
            direction = next_direction(&br);
            break;
        default:
            exit(-1);
    }

    // direction = ((nx / 32 + ny / 32)) % 2;

    direction = direction % 4;

    index1 = indicies[orderings[4 * direction + 0]];
    index2 = indicies[orderings[4 * direction + 1]];
    index3 = indicies[orderings[4 * direction + 2]];
    index4 = indicies[orderings[4 * direction + 3]];

    // Get the data from the cell, oriented with the top cells pointing towards [direction] 
    // and the bottom cells pointing away.
    tl = data_buffer[index1];
    tr = data_buffer[index2];
    bl = data_buffer[index3];
    br = data_buffer[index4];

    // Get the corresponding densities and states
    tl_density = material_data[tl.material].density;
    tr_density = material_data[tr.material].density;
    bl_density = material_data[bl.material].density;
    br_density = material_data[br.material].density;

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
