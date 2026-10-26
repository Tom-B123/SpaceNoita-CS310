#include "engine.h"
#include "app.h"
#include "box2d/box2d.h"
#include "box2d/id.h"
#include "materials.h"
#include <GL/gl.h>

bool* is_stale = new bool[WORLD_WIDTH * WORLD_HEIGHT];
int MAX_UPDATES2 = MAX_UPDATES * MAX_UPDATES;

int object_count = 0;

float rand_range(float start, float end) {
    // Source - https://stackoverflow.com/a/686373
    return start + static_cast <float> (rand()) /( static_cast <float> (RAND_MAX/(end-start)));
}

// Simulated rigidbody objects
std::vector<RigidbodyObject> objects = std::vector<RigidbodyObject>(MAX_OBJECTS);
// Object data to be drawn
std::vector<BodyInstance> raw_objects = std::vector<BodyInstance>(MAX_OBJECTS);

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
    spawn_material(data_buffer,dirty,x, y,material_id, 0,0);
}


// Return the index of the largest of four floats, using 3 conditionals
int largest_of_four(float A, float B, float C, float D) {
    // If the total momentum is beneath a threshold, we stop moving.
    // Else return the cell with the biggest momentum
    if (A >= B) {
        if (A >= C) {
            if (A >= D) {
                // A is largest
                return 0;
            }   else {
                // D is largest
                return 3;
            }
        }   else {
            if (C >= D) {
                // C is largest
                return 2;
            }   else {
                // D is largest
                return 3;
            }
        }
        // B > A case
    }   else {
        if (B >= C) {
            if (B >= D) {
                // A is largest
                return 1;
            }   else {
                // D is largest
                return 3;
            }
        }   else {
            if (C >= D) {
                // C is largest
                return 2;
            }   else {
                // D is largest
                return 3;
            }
        }
    }
}

const float GRAVITY_MAGNITUDE = 1;
const float FRICTION = 0.9;
/**
 *  Apply friction and gravity to the particle's velocities.
 *  Reset the frame_velocity, i.e. how much the particle has moved this frame
 *  x_vel of 1 means move facing west by 1 update
 *  @param data the particle that is being updated.
 */
void update_forces(DataPoint* data, b2BodyId body, float s_rotation, float c_rotation) {

    data->frame_x_vel = 0;
    data->frame_y_vel = 0;
    
    // apply gravity
    data->x_vel += s_rotation;
    data->y_vel += c_rotation;
    // clamp speed to MAX_UPDATES / frame
    float magnitude = data->x_vel * data->x_vel + data->y_vel * data->y_vel;
    if (magnitude > MAX_UPDATES2) {
        data->x_vel = data->x_vel * MAX_UPDATES / std::sqrt(magnitude);
        data->y_vel = data->y_vel * MAX_UPDATES / std::sqrt(magnitude);
    }
    // apply friction
    data->x_vel *= FRICTION;
    data->y_vel *= FRICTION;
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
    if (x_vel > 0.1) {
        data->frame_x_vel++;
        // If we have exceeded the velocity, see if we should move anyway by randomly rolling and comparing to the difference in velocity
        if (data->frame_x_vel > data->x_vel) {
            data->frame_x_vel = data->x_vel;
            if (rand_range(0,1) < data->frame_x_vel - data->x_vel) {
                return -1;
            }
        }
        return 3;
    }
    if (x_vel < -0.1) {
        data->frame_x_vel--;
        if (data->frame_x_vel < data->x_vel) {
            data->frame_x_vel = data->x_vel;
            if (rand_range(0,1) < -data->frame_x_vel + data->x_vel) {
                return -1;
            }
        }
        return 1;
    }
    if (y_vel > 0.1) {
        data->frame_y_vel++;
        if (data->frame_y_vel > data->y_vel) {
            data->frame_y_vel = data->y_vel;
            if (rand_range(0,1) < data->frame_y_vel - data->y_vel) {
                return -1;
            }
        }
        return 0;
    }
    if (y_vel < -0.1) {
        data->frame_y_vel--;
        data->frame_y_vel++;
        if (data->frame_y_vel < data->y_vel) {
            data->frame_y_vel = data->y_vel;
            if (rand_range(0,1) < -data->frame_y_vel + data->y_vel) {
                return -1;
            }
        }
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

    // See if this works for multiple steps / frame
    if (swapped) {
        dirty[index1] = 0;
        dirty[index2] = 0;
        dirty[index3] = 0;
        dirty[index4] = 0;
    }   else {
        is_stale[index1] = true;
        is_stale[index2] = true;
        is_stale[index3] = true;
        is_stale[index4] = true;
    }
    return data_buffer;
}


/**
 *  Computes the next value for the data_buffer, at the given cell position.
 *  Mimics the GPU code for a single thread
 *  @param 
 */
DataPoint* simple_margolus_update(Material* material_data, DataPoint* data_buffer, char* dirty, long step) {

    b2Rot rotation = b2Body_GetRotation(objects[0].id);
    // rotation in radians
    float rotation_r = b2Rot_GetAngle(rotation);

    float c_rotation = GRAVITY_MAGNITUDE * cos(rotation_r);
    float s_rotation = GRAVITY_MAGNITUDE * sin(rotation_r);

    // Update the forces for all pixels. If the pixel is stale, don't bother
    for (int y = 0; y < WORLD_HEIGHT; y++) {
        for (int x = 0; x < WORLD_WIDTH; x++) {

            if (is_stale[y * WORLD_WIDTH + x] && dirty[y * WORLD_WIDTH + x] < STALE_TIME) dirty[y * WORLD_WIDTH + x] ++;
            update_forces(&data_buffer[y * WORLD_WIDTH + x],objects[0].id,s_rotation,c_rotation);
            // if (dirty[y * (WORLD_WIDTH) + x] >= STALE_TIME) {
            //     continue;
            // }
        }
    }

    bool resolved_velocity = false;
    int i = 0;
    // Update i times.
    while (!resolved_velocity && i < MAX_UPDATES) {
        resolved_velocity = true;
        for (int y = 0; y < WORLD_HEIGHT / 2; y++) {
            for (int x = 0; x < WORLD_WIDTH / 2; x++) {

                float force_update = rand_range(0, 1);
                int chunk_size = WORLD_WIDTH;

                // Get the normalised x,y position for the top left corner
                int nx = 2*x + ((step+i)%2);
                int ny = 2*y + ((step+i)%2);

                // This isn't really working, without a solid rock barrier we get corruption at the world edge
                if (nx >= WORLD_WIDTH-1 || nx < 0 || ny >= WORLD_HEIGHT-1 || ny < 0) continue;

                // If there are no dirty cells we can skip
                if (
                        dirty[ny * (chunk_size) + nx] >= STALE_TIME &&                                      // [' ]
                        dirty[ny * (chunk_size) + (nx+1) % chunk_size] >= STALE_TIME &&                     // [ ']
                        dirty[(((ny+1)%chunk_size) * (chunk_size)) + nx] >= STALE_TIME &&                   // [. ]
                        dirty[(((ny+1)%chunk_size) * (chunk_size)) + (nx+1) % chunk_size] >= STALE_TIME     // [ .]
                ) {
                    // Occasionally we don't skip
                    if (force_update > 0.9993) {
                        dirty[ny * (chunk_size) + nx] = 0;
                        dirty[ny * (chunk_size) + (nx+1) % chunk_size] = 0;
                        dirty[(((ny+1)%chunk_size) * (chunk_size)) + nx] = 0;
                        dirty[(((ny+1)%chunk_size) * (chunk_size)) + (nx+1) % chunk_size] = 0;
                    }   else {
                        continue;
                    }
                }

                int indicies[] = {
                    ny * (chunk_size) + nx,                                      // [' ]
                    ny * (chunk_size) + (nx+1) % chunk_size,                     // [ ']
                    (((ny+1)%chunk_size) * (chunk_size)) + nx,                   // [. ]
                    (((ny+1)%chunk_size) * (chunk_size)) + (nx+1) % chunk_size   // [ .]
                };


                // Which direction is "up"? 0 -> North, 1 -> East etc.
                // We assume direction is 0, then calculate what it should be for this cell
                int direction = 0;

                int index1 = ny * (chunk_size) + nx;
                int index2 = ny * (chunk_size) + (nx+1)%chunk_size;
                int index3 = ((ny+1)%chunk_size) * (chunk_size) + nx;
                int index4 = ((ny+1)%chunk_size) * (chunk_size) + (nx+1) % chunk_size;

                // Get the data from the cell, oriented with the top cells pointing towards [direction] 
                // and the bottom cells pointing away.
                DataPoint tl = data_buffer[index1];
                DataPoint tr = data_buffer[index2];
                DataPoint bl = data_buffer[index3];
                DataPoint br = data_buffer[index4];

                // Apply changed forces
                int tl_density = material_data[tl.material].density;
                int tr_density = material_data[tr.material].density;
                int bl_density = material_data[bl.material].density;
                int br_density = material_data[br.material].density;

                // Calculate the momentum for each cell (sqaured length of velocity * density)
                // The highest momentum determines which direction this cell should update in.
                // Should account for the frame distance travelled
                float momentum_tl =  (tl.x_vel - tl.frame_x_vel) * (tl.x_vel - tl.frame_x_vel)
                    +   (tl.y_vel - tl.frame_y_vel) * (tl.y_vel-tl.frame_y_vel);
                float momentum_tr =   (tr.x_vel - tr.frame_x_vel) * (tr.x_vel - tr.frame_x_vel)
                    +   (tr.y_vel - tr.frame_y_vel) * (tr.y_vel-tr.frame_y_vel);
                float momentum_bl =   (bl.x_vel - bl.frame_x_vel) * (bl.x_vel - bl.frame_x_vel)
                    +   (bl.y_vel - bl.frame_y_vel) * (bl.y_vel-bl.frame_y_vel);
                float momentum_br =   (br.x_vel - br.frame_x_vel) * (br.x_vel - br.frame_x_vel)
                    +  (br.y_vel - br.frame_y_vel) * (br.y_vel-br.frame_y_vel);
                // Move cells many times per update, moving as the dominant is moving for this update. Don't know initially how many times a given grid will need to update

                // Get the highest momentum and get the right direction from this.
                int dominant = largest_of_four(momentum_tl, momentum_tr, momentum_bl, momentum_br);
                float grid_momentum = -1;
                switch (dominant) {
                    case 0: grid_momentum = momentum_tl; break;
                    case 1: grid_momentum = momentum_tr; break;
                    case 2: grid_momentum = momentum_bl; break;
                    case 3: grid_momentum = momentum_br; break;
                }
                if (grid_momentum < 1) continue; 

                // std::cout << "Frame 1: |" << tl.x_vel << ":" << tl.frame_x_vel << "," << tl.y_vel << ":" << tl.frame_y_vel << " , " << tr.x_vel <<":" << tr.frame_x_vel << "," << tr.y_vel << ":" << tr.frame_y_vel << "|" << std::endl
                //           << "Frame 1: |" << bl.x_vel << ":" << bl.frame_x_vel << "," << bl.y_vel << ":" << bl.frame_y_vel << " , " << br.x_vel <<":" << br.frame_x_vel << "," << tr.y_vel << ":" << tr.frame_y_vel << "|" << std::endl;

                resolved_velocity = false;

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


                // std::cout << "Frame 2: |" << tl.x_vel << ":" << tl.frame_x_vel << "," << tl.y_vel << ":" << tl.frame_y_vel << " , " << tr.x_vel <<":" << tr.frame_x_vel << "," << tr.y_vel << ":" << tr.frame_y_vel << "|" << std::endl
                //           << "Frame 2: |" << bl.x_vel << ":" << bl.frame_x_vel << "," << bl.y_vel << ":" << bl.frame_y_vel << " , " << br.x_vel <<":" << br.frame_x_vel << "," << tr.y_vel << ":" << tr.frame_y_vel << "|" << std::endl;

                // tl, tr, bl or br have been updated by calling next_direction (potentially all 4) so this needs to be written BACK to 
                // the data_buffer
                data_buffer[index1] = tl;
                data_buffer[index2] = tr;
                data_buffer[index3] = bl;
                data_buffer[index4] = br;

                // Update the data_buffer by 1 frame and update the tl tr bl br pointers
                data_buffer = update(material_data, data_buffer, dirty, direction,nx,ny,chunk_size);

            }
        }
        i++;
    }

    return data_buffer;
}




void add_body(RigidbodyObject bo) {
    if (object_count >= MAX_OBJECTS) return; 

    objects[object_count] = bo;
    raw_objects[object_count++] = bo.data;
}

void add_body(BodyInstance bi, b2BodyId id) {
    RigidbodyObject bo = {bi,id};
    add_body(bo);
}

void add_body(b2WorldId worldId, bool is_dynamic, float x_pos, float y_pos, float x_scale, float y_scale) {
    b2BodyDef bodyDef = b2DefaultBodyDef();
    if (is_dynamic) bodyDef.type = b2_dynamicBody;
    bodyDef.position = (b2Pos){x_pos, y_pos};
    b2BodyId bodyId = b2CreateBody(worldId, &bodyDef);

    b2Polygon box = b2MakeBox(x_scale, y_scale);
    b2ShapeDef shapeDef = b2DefaultShapeDef();
    shapeDef.density = 1.0f;
    shapeDef.material.friction = 0.3f;

    b2CreatePolygonShape(bodyId, &shapeDef, &box);

    BodyInstance bi = {x_pos,y_pos,2 * x_scale,2 * y_scale,0,rand_range(0,1),rand_range(0, 1),rand_range(0, 1)};
    RigidbodyObject bo = {bi,bodyId};
    add_body(bo);
}

b2WorldId box2d_init() {
    b2WorldDef worldDef = b2DefaultWorldDef();
    worldDef.gravity = (b2Vec2){0.0f, 0.0f};
    b2WorldId worldId = b2CreateWorld(&worldDef);

    add_body(worldId, true, 30.0f, 0.0f, 100.0f, 100.0f);
    b2Body_SetAngularVelocity(objects[0].id, M_PI / 16);

    return worldId;
}

void box2d_update(b2WorldId worldId) {
    b2World_Step(worldId, 1.0f / (float)TARGET_FPS, B2D_SUBSTEP_COUNT);
    for (int i = 0; i < object_count; i++) {
        b2Pos position = b2Body_GetPosition(objects[i].id);
        b2Rot rotation = b2Body_GetRotation(objects[i].id);

        objects[i].data.angle = b2Rot_GetAngle(rotation);
        objects[i].data.x_pos = position.x;
        objects[i].data.y_pos = -position.y;
        raw_objects[i].angle = b2Rot_GetAngle(rotation);
        raw_objects[i].x_pos = position.x;
        raw_objects[i].y_pos = -position.y;
    }
}

void box2d_bind() {
    glBufferSubData(GL_ARRAY_BUFFER, 0, object_count * sizeof(BodyInstance), raw_objects.data());
}
void box2d_draw() {
    glDrawArraysInstanced(GL_TRIANGLE_FAN, 0, 4, object_count);
}
