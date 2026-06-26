#include "barebones_engine.h"
#include "app.h"

BarebonesEngine::BarebonesEngine(int n_world_width, int n_world_height) {
    world_data = init_buf(world_width,world_height,'?');
    world_width = n_world_width;
    world_height = n_world_height;
}

void BarebonesEngine::mutate(buffer render_buffer) {

}
