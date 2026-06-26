#include "engine.h"
#include "GLFW/glfw3.h"
#include "app.h"
#include "barebones_engine.h"
#include "buffer.h"
#include "chunk_manager.h"
#include "graphics.h"
#include <chrono>

Engine::Engine(int world_width, int world_height,
    GameWindow* n_window) : 
    window(n_window),
    iteration(0)
{
    // Tell the kernel to use 2x2 cells in margolous neighbourhood
    n_width = 2;
    n_height = 2;

    // All inputs initially not pressed
    input_state = {
        false,
        false,
        false,
        false
    };
    // Initialise the render buffer to all invalid cells
    render_buffer = init_render_buf(world_width,world_height,'?');
    barebones_engine = new BarebonesEngine(world_width,world_height);
}

int Engine::main_loop() {

    long target_fps = 60;
    long target_frame_time = 1000000 / target_fps;

    // Main loop, 
    while (window->is_open()) {
        auto start = std::chrono::high_resolution_clock::now();

        barebones_engine->mutate(render_buffer);

        // Display the new render buffer
        window->update_texture(render_buffer.data);

        window->refresh();
        window->draw();

        // Take user inputs
        input();
        auto end = std::chrono::high_resolution_clock::now();

        long delta = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();

        if (delta < target_frame_time) Sleep((target_frame_time - delta) / 1000);
    }

    std::cout << "Exited main loop" << std::endl;
    window->close();
    std::cout << "Window closed" << std::endl;

    return 0;
}

void Engine::keyCallback(GLFWwindow* window, int key, int scancode, int action, int mode) {
    Engine* engine = (Engine*)glfwGetWindowUserPointer(window);

    if (!engine) { return; }

    bool is_pressed = (action == GLFW_PRESS || action == GLFW_REPEAT);

    switch(key) {
        case GLFW_KEY_W: engine->input_state.camera_up    = is_pressed; break;
        case GLFW_KEY_A: engine->input_state.camera_right = is_pressed; break;
        case GLFW_KEY_S: engine->input_state.camera_down  = is_pressed; break;
        case GLFW_KEY_D: engine->input_state.camera_left  = is_pressed; break;
    }
}

void Engine::input() {
    // chunk_manager.input(input_state);
}
