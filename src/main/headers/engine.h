#include "materials.h"
// Takes two positions and swaps their data in the databuffer
void swap_cells(int x1, int y1, int x2, int y2,DataPoint* data_buffer);

// Sets the material at a given position, replacing air
void spawn_material(int x, int y, char material_id, DataPoint* data_buffer);

DataPoint* simple_margolus_update(Material* material_data, DataPoint* data_buffer, long step,
                            int x, int y);
