#include "engine.h"
#include "app.h"
#include "materials.h"
#include <algorithm>
#include <cmath>
#include <vector>

const int STALE_TIME = 31;

void swap_cells(DataPoint* data_buffer, char* dirty, int indexA, int indexB) {
    DataPoint tmp = data_buffer[indexA];
    data_buffer[indexA] = data_buffer[indexB];
    data_buffer[indexB] = tmp;
}

// Takes two positions and swaps their data in the databuffer
void swap_cells(DataPoint* data_buffer, char* dirty, int x1, int y1, int x2, int y2) {
    swap_cells(
            data_buffer,
            dirty,
            (y1%WORLD_HEIGHT) * WORLD_WIDTH + (x1%WORLD_WIDTH), 
            (y2%WORLD_HEIGHT) * WORLD_WIDTH + (x2%WORLD_WIDTH)
    );
}

// Spawns material, setting the initial velocity
void spawn_material(DataPoint* data_buffer, char* dirty, int x, int y, char material_id, float x_vel, float y_vel) {
    if (data_buffer[y * WORLD_WIDTH + x].material == ' ') {
        data_buffer[y * WORLD_WIDTH + x].material = material_id;
        data_buffer[y * WORLD_WIDTH + x].x_vel = x_vel;
        data_buffer[y * WORLD_WIDTH + x].y_vel = y_vel;
        data_buffer[y * WORLD_WIDTH + x].frame_x_vel = 0;
        data_buffer[y * WORLD_WIDTH + x].frame_y_vel = 0;
        data_buffer[y * WORLD_WIDTH + x].chunk_id = 0;
        data_buffer[y * WORLD_WIDTH + x].body_id = 0;
        dirty[y * WORLD_WIDTH + x] = 0; 
    }
}
// Sets the material at a given position, repintacing air
void spawn_material(DataPoint* data_buffer, char* dirty, int x, int y, char material_id) {
    spawn_material(data_buffer,dirty,x, y,material_id, 0,-1);
}

// Return the index of the largest of four floats, using 3 conditionals
int largest_of_four(float A, float B, float C, float D) {
    // If the total momentum is beneath a threshold, we stop moving.
    if (A + B + C + D < 2) {
        return -1;
    }
    // Else return the cell with the biggest momentum
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

/**
 *  Apply friction and gravity to the particle's velocities.
 *  Reset the frame_velocity, i.e. how much the particle has moved this frame
 *  x_vel of 1 means move facing west by 1 update
 *  @param data the particle that is being updated.
 */
void update_forces(DataPoint* data) {
    data->frame_x_vel = 0;
    data->frame_y_vel = 0;
    // apply friction to x_velocity
    data->x_vel = data->x_vel * 0.9;
    // apply friction + gravity to y_velocity. Should reach natuaral terminal velocity asp
    data->y_vel = 1 + data->y_vel * 0.9;
}

/**
 *  Choose the next direction for the next update, based on the dominant particle's velocity (one with highest velocity^2 * mass^2)
 *  This should happen as many times per frame as necessary until everything has a |frame_x_vel| >= |x_vel| vica versa
 *  @param data the dominat particle that is being updated.
 */
int next_direction(DataPoint* data) {

    // Naive approach is move x fully then y fully, a-la minecraft entity physics, may prove more reliable / intuitive to move diagonally a-la bresenhams, 
    // depends on how noticable the artifacting is.

    // Subtract the currently moved steps from the remaining velocity.
    float x_vel = data->x_vel - data->frame_x_vel;
    float y_vel = data->y_vel - data->frame_y_vel;

    // Move in direction of velocity
    if (x_vel > 1) {
        data->frame_x_vel++;
        if (data->frame_x_vel > data->x_vel) data->frame_x_vel = data->x_vel;
        return 3;
    }
    if (x_vel < -1) {
        data->frame_x_vel--;
        if (data->frame_x_vel < data->x_vel) data->frame_x_vel = data->x_vel;
        return 1;
    }
    if (y_vel > 1) {
        data->frame_y_vel++;
        if (data->frame_y_vel > data->y_vel) data->frame_y_vel = data->y_vel;
        return 0;
    }
    if (y_vel < -1) {
        data->frame_y_vel--;
        if (data->frame_y_vel < data->y_vel) data->frame_y_vel = data->y_vel;
        return 2;
    }

    // Move nowhere
    return -1;
}

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

DataPoint* update(Material* material_data, DataPoint* data_buffer, char* dirty, int direction, int nx, int ny, int chunk_size) {
    // Basecase: no velocities in the cell so just return
    if (direction == -1) {
        return data_buffer;
    }
    // Else, update in the given direction
    direction = direction % 4;

    int indicies[] = {
        ny * (chunk_size) + nx,                                      // [' ]
        ny * (chunk_size) + (nx+1) % chunk_size,                     // [ ']
        (((ny+1)%chunk_size) * (chunk_size)) + nx,                   // [. ]
        (((ny+1)%chunk_size) * (chunk_size)) + (nx+1) % chunk_size   // [ .]
    };


    // Which direction is "up"? 0 -> North, 1 -> East etc.
    // We assume direction is 0, then calculate what it should be for this cell

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

    bool is_x = direction == 1 || direction == 3;

    // Get the corresponding densities and states
    int tl_density = material_data[tl.material].density;
    int tr_density = material_data[tr.material].density;
    int bl_density = material_data[bl.material].density;
    int br_density = material_data[br.material].density;

    int tl_state = material_data[tl.material].state;
    int tr_state = material_data[tr.material].state;
    int bl_state = material_data[bl.material].state;
    int br_state = material_data[br.material].state;

    bool swapped = false;

    /** Defines the rules for when to swap cells **/

    // Vertical rules, [' ] -> [. ] & [ '] -> [ .] (and [''] -> [..])
    if (tl_density > bl_density) {
        if ((tl_state == POWDER || tl_state == LIQUID || tl_state == GAS) && bl_state != SOLID) {
            swap_cells(data_buffer,dirty,index1,index3);
            swapped = true;
        }
    }
    if (tr_density > br_density) {
        if ((tr_state == POWDER || tr_state == LIQUID || tr_state == GAS) && br_state != SOLID) {
            swap_cells(data_buffer,dirty,index2,index4);
            swapped = true;
        }
    }
    // Diagonal rules, [: ] -> [..] & [ :] -> [..] 
    if (!swapped && tl_density > br_density) {
        if ((tl_state == POWDER || tl_state == LIQUID || tl_state == GAS) && br_state != SOLID) {
            swap_cells(data_buffer,dirty,index1,index4);
            swapped = true;
        }
    }
    if (!swapped && tr_density > bl_density) {
        if ((tr_state == POWDER || tr_state == LIQUID || tr_state == GAS) && bl_state != SOLID) {
            swap_cells(data_buffer,dirty,index2,index3);
            swapped = true;
        }
    }
    // Horizontal rules, [. ] -> [ .] & [:.] -> [.:] (and their mirrors)
    if (!swapped && tl_density > tr_density) {
        if ((tl_state == LIQUID || tl_state == GAS) && tr_state != SOLID) {
            swap_cells(data_buffer,dirty,index1,index2);
            swapped = true;
        }
    }
    if (!swapped && bl_density > br_density) {
        if ((bl_state == LIQUID || bl_state == GAS) && br_state != SOLID) {
            swap_cells(data_buffer,dirty,index3,index4);
            swapped = true;
        }
    }
    if (!swapped && tr_density > tl_density) {
        if ((tr_state == LIQUID || tr_state == GAS) && tl_state != SOLID) {
            swap_cells(data_buffer,dirty,index2,index1);
            swapped = true;
        }
    }
    if (!swapped && br_density > bl_density) {
        if ((br_state == LIQUID || br_state == GAS) && bl_state != SOLID) {
            swap_cells(data_buffer,dirty,index4,index3);
            swapped = true;
        }
    }

    if (swapped) {
        dirty[index1] = 0;
        dirty[index2] = 0;
        dirty[index3] = 0;
        dirty[index4] = 0;
    }   else {
        if (dirty[index1] < STALE_TIME) dirty[index1] ++;
        if (dirty[index2] < STALE_TIME) dirty[index2] ++;
        if (dirty[index3] < STALE_TIME) dirty[index3] ++;
        if (dirty[index4] < STALE_TIME) dirty[index4] ++;
    }
    return data_buffer;
}

/**
 *  Computes the next value for the data_buffer, at the given cell position.
 *  Mimics the GPU code for a single thread
 *  @param 
 */
DataPoint* simple_margolus_update(Material* material_data, DataPoint* data_buffer, char* dirty, long step) {

    for (int y = 0; y < WORLD_HEIGHT / 2; y++) {
        for (int x = 0; x < WORLD_WIDTH / 2; x++) {

            int chunk_size = WORLD_WIDTH;

            // Get the normalised x,y position for the top left corner
            int nx = 2*x + (step%2);
            int ny = 2*y + (step%2);

            if (ny >= WORLD_HEIGHT - 1) { continue; }

            // If there are no dirty cells we can skip
            if (dirty[ny * (chunk_size) + nx] >= STALE_TIME &&                                      // [' ]
                dirty[ny * (chunk_size) + (nx+1) % chunk_size] >= STALE_TIME &&                     // [ ']
                dirty[(((ny+1)%chunk_size) * (chunk_size)) + nx] >= STALE_TIME &&                   // [. ]
                dirty[(((ny+1)%chunk_size) * (chunk_size)) + (nx+1) % chunk_size] >= STALE_TIME     // [ .]
            ) continue;

            int indicies[] = {
                ny * (chunk_size) + nx,                                      // [' ]
                ny * (chunk_size) + (nx+1) % chunk_size,                     // [ ']
                (((ny+1)%chunk_size) * (chunk_size)) + nx,                   // [. ]
                (((ny+1)%chunk_size) * (chunk_size)) + (nx+1) % chunk_size   // [ .]
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

            // Call this once per update
            update_forces(&tl);
            update_forces(&tr);
            update_forces(&bl);
            update_forces(&br);

            // Apply changed forces
            int tl_density = material_data[tl.material].density;
            int tr_density = material_data[tr.material].density;
            int bl_density = material_data[bl.material].density;
            int br_density = material_data[br.material].density;

            // Calculate the momentum for each cell (sqaured length of velocity * density)
            // The highest momentum determines which direction this cell should update in.
            // Should account for the frame distance travelled
            float momentum_tl = tl_density * tl_density * (tl.x_vel - tl.frame_x_vel) * (tl.x_vel - tl.frame_x_vel)
                + tl_density * tl_density * (tl.y_vel - tl.frame_y_vel) * (tl.y_vel-tl.frame_y_vel);
            float momentum_tr = tr_density * tr_density * (tr.x_vel - tr.frame_x_vel) * (tr.x_vel - tr.frame_x_vel)
                + tr_density * tr_density * (tr.y_vel - tr.frame_y_vel) * (tr.y_vel-tr.frame_y_vel);
            float momentum_bl = bl_density * bl_density * (bl.x_vel - bl.frame_x_vel) * (bl.x_vel - bl.frame_x_vel)
                + bl_density * bl_density * (bl.y_vel - bl.frame_y_vel) * (bl.y_vel-bl.frame_y_vel);
            float momentum_br = br_density * br_density * (br.x_vel - br.frame_x_vel) * (br.x_vel - br.frame_x_vel)
                + br_density * br_density * (br.y_vel - br.frame_y_vel) * (br.y_vel-br.frame_y_vel);

            // Move cells many times per update, moving as the dominant is moving for this update. Don't know initially how many times a given grid will need to update

            // Get the highest momentum and get the right direction from this.
            int dominant = largest_of_four(momentum_tl, momentum_tr, momentum_bl, momentum_br);

            // Pick a direction based on which if the 4 cells has the most "momentum" (velocity * mass)
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
                // If we have 0 velocity, we don't need to update
                default:
                    continue;
            }

            // tl, tr, bl or br have been updated by calling next_direction (potentially all 4) so this needs to be written BACK to 
            // the data_buffer
            data_buffer[index1] = tl;
            data_buffer[index2] = tr;
            data_buffer[index3] = bl;
            data_buffer[index4] = br;

            nx = 2*x + (step%2);
            ny = 2*y + (step%2);

            data_buffer = update(material_data, data_buffer, dirty, direction,nx,ny,chunk_size);

            tl = data_buffer[index1];
            tr = data_buffer[index2];
            bl = data_buffer[index3];
            br = data_buffer[index4];
        }
    }

    return data_buffer;
}
