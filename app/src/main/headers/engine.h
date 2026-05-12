#ifndef ENGINE_H
#define ENGINE_H
// Handle the game update loop for the player and the world
#include "app.h"
#include "chunk_manager.h"
#include "cl_setup.h"
#include "graphics.h"

// Represents the position of the camera with respect to 0,0.
class Engine {
    private:
        ChunkManager chunk_manager;
        CL* cl;
        GameWindow* window;
        // Current simulation step
        int iteration;

        // Neighbourhood size, default is 2x2
        int n_width;
        int n_height;


    public:
        Engine(int world_width, int world_height, 
                CL* n_cl,GameWindow* n_window);
        void update();
        int main_loop();

};

#endif
