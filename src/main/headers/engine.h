#include "materials.h"
#include "app.h"
// Takes two positions and swaps their data in the databuffer
void swap_cells(DataPoint* data_buffer, char* dirty, int indexA, int indexB);
void swap_cells(DataPoint* data_buffer, char* dirty, int x1, int y1, int x2, int y2);

// Sets the material at a given position, replacing air
void spawn_material(DataPoint* data_buffer, char* dirty, int x, int y, char material_id, float x_vel, float y_vel);
void spawn_material(DataPoint* data_buffer, char* dirty, int x, int y, char material_id);
/**
 *  Update the simulation using margolus neighbourhoods
 *  @param material_data Information about each material loaded from the materials.json file
 *  @param data_buffer The world state
 *  @param dirty Marks which cells need to be updated and which can be left
 *  @param step Simulation step
 *  
 */
DataPoint* simple_margolus_update(Material* material_data, DataPoint* data_buffer,char* dirty, long step);


/**
 *  Stores the data for rendering one quad. 
 *  Should use a texture that is updated using the falling sand logic
 */
struct BodyInstance {
    float x_pos; float y_pos;
    float x_scale; float y_scale;
    float angle; 
    float r; float g; float b;
};

/**
 *  Stores all the necessary data for a rigidbody object
 */
struct RigidbodyObject {
    BodyInstance data;
    b2BodyId id;
};

/**
 *  Add a new body to the world
 */
void add_body(RigidbodyObject bo);
/**
 *  Add a new body to the world using the instance data and B2D ID
 */
void add_body(BodyInstance bi, b2BodyId id);
/**
 *  Create a new b2d object and instance data using the given params. Add this to the simulation.
 */
void add_body(b2WorldId worldId, bool is_dynamic, float x_pos, float y_pos, float x_scale, float y_scale);
/**
 *  Create the initial world
 */
b2WorldId box2d_init();

/**
 *  Update the world and the instance data
 */
void box2d_update(b2WorldId worldId);


/**
 *  Bind the vertex data to be drawn
 */
void box2d_bind();
/**
 *  Draw the box2d objects
 */
void box2d_draw();
