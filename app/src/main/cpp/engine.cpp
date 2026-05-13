#include "engine.h"
#include "GLFW/glfw3.h"
#include "buffer.h"
#include "chunk_manager.h"
#include "graphics.h"

Engine::Engine(int world_width, int world_height,
        CL* n_cl, GameWindow* n_window) : 
    cl(n_cl),
    window(n_window),
    chunk_manager(ChunkManager(world_width,world_height,n_cl)),
    iteration(0)
{
    n_width = 2;
    n_height = 2;

    input_state = {
        false,
        false,
        false,
        false
    };
}

void Engine::update() {

}

int Engine::main_loop() {
    buffer render_buf = chunk_manager.get_render_buffer();

    // Update n times per tick
    int iteration = 0;

    // Set Process kernel arguments
    cl->setArg(5, render_buf.width,cl->process_kernel);
    cl->setArg(6, render_buf.height,cl->process_kernel);
    cl->setArg(7, n_width,cl->process_kernel);
    cl->setArg(8, n_height,cl->process_kernel);

    // Set Render kernel arguments
    cl->setArg(6, render_buf.width,cl->render_kernel);
    cl->setArg(7, render_buf.height,cl->render_kernel);


    // Main loop, 
    char materials[] = {'S','O','W'};


    while (window->is_open()) {

        chunk_manager.update_chunks(cl);

        render_buf = chunk_manager.get_render_buffer();

        window->update_texture(render_buf.data);

        window->refresh();
        window->draw();

        input();
    }

    window->close();

    Sleep(2000);

    delete[](render_buf.data);
    cl->free();

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
    chunk_manager.input(input_state);
}
