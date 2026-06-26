#ifndef BAREBONE_H
#define BAREBONE_H
#include "buffer.h"
#include "app.h"
class BarebonesEngine {
    private:
        buffer world_data;
        int world_width;
        int world_height;
    public:
        BarebonesEngine(int world_width, int world_height);
        void free() {
            std::cout << "Freed barbones engine" << std::endl;
            free_buffer(world_data);

        }
        // step the world by 1 tick and mutate the render buffer to reflect this.
        void mutate(buffer render_buffer);
};

#endif
