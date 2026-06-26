#ifndef ENGINE_H
#define ENGINE_H
// Handle the game update loop for the player and the world
#include "barebones_engine.h"
#include "graphics.h"
#include "app.h"
#include "chunk_manager.h"
#include "cl_setup.h"

// Give input types rather than exact keys, so can be re-bound easier

// Represents the position of the camera with respect to 0,0.
class Engine {
    private:
        GameWindow* window;
        // Current simulation step
        int iteration;

        // Neighbourhood size, default is 2x2
        int n_width;
        int n_height;
        buffer render_buffer;
        BarebonesEngine* barebones_engine;
        
        InputState input_state;
    

    public:
        static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mode);
        Engine(int world_width, int world_height, 
                GameWindow* n_window);
        ~Engine() {
            std::cout << "Engine freed" << std::endl;
            free_buffer(render_buffer);
            (*barebones_engine).free();
            // In your Engine destructor:
            if (glfwGetCurrentContext() != nullptr) {
                // Only do GLFW stuff if context still exists
                glfwSetWindowUserPointer(window->get_window(), nullptr);
            }
        }
        void update();
        void input();
        int main_loop();
};

#endif
