#ifndef MATERIALS 
#define MATERIALS
#include "util.h"

#include <stdlib.h>
// Information about a given material, from materials.json
// NOTE for code that should be OpenCL compatable, structs should always be used
// over classes, as OpenCL essentially has base C features only
struct Material {
    char name[128];
    char colour[8];
    int density;
    int state;
};

// The data stored in each cell
struct DataPoint {
    char material;      // The material type stored
    float x_vel;        // Velocity
    float y_vel;
    float frame_x_vel;  // How much the particle has moved this frame
    float frame_y_vel;
    int chunk_id;       // Assuming these will be useful in future
    int body_id;
};

int load_materials(float* colours,Material* material_data);
#endif
