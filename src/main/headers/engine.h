#include "materials.h"
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
