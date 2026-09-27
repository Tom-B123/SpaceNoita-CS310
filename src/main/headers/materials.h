#ifndef MATERIALS 
#define MATERIALS
#include "util.h"

#include <stdlib.h>
// Information about a given material, from materials.json
struct Material {
    char name[128];
    char colour[8];
    int density;
    int state;
};

int load_materials(float* colours,Material* material_data);
#endif
