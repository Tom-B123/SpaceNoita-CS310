#include "materials.h"
#include <algorithm>
#include <bitset>
#include <map>
#include <chrono>
#include <thread>
#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include "app.h"


#include <cstdlib>

#include <fstream>
#include <iostream>

#include"util.h"
#include"engine.h"
bool debug_display = false;

Material* material_data = new Material[256];
float* colours = new float[256 * 3];


DataPoint* init_data_buffer(int w, int h) {
    DataPoint* data_buffer = new DataPoint[w*h];
    for (int y = 0; y < h; y++) {
        for (int x = 0; x < w; x++) {
            data_buffer[y * w + x] = {
                'S',
                false
            };
        }
    }
    return data_buffer;
}

char* update_step(char* render_buffer,DataPoint* data_buffer,char* dirty, long step,
                    int num_updates, float player_x, float player_y) {

    spawn_material(data_buffer, dirty, WORLD_WIDTH / 2, WORLD_HEIGHT / 2, 'S');

    data_buffer = simple_margolus_update(material_data,data_buffer,dirty,step);

    for (int y = 0; y < WORLD_HEIGHT; y++) {
        for (int x = 0; x < WORLD_WIDTH; x++) {
            if (!debug_display) render_buffer[y*WORLD_WIDTH + x] = data_buffer[y * WORLD_WIDTH + x].material;
            if (debug_display)  render_buffer[y*WORLD_WIDTH + x] = dirty[y * WORLD_WIDTH + x];
        }
    }

    // render_buffer[((int)player_y % WORLD_HEIGHT)  * WORLD_WIDTH + ((int)player_x % WORLD_WIDTH)] = 'P';

    return render_buffer;
}

// Use keypressed and keyreleased to track which keys are currently held down
#define NUM_KEYS 349

bool key_state[NUM_KEYS];

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (key < 0 || key >= NUM_KEYS) {
        std::cout << "Invalid key error" << std::endl;
    }
    // Should handle unknown key (-1)
    // if (key == GLFW_KEY_UNKNOW) { return; }
    if(action == GLFW_PRESS) {
        key_state[key] = true;
        if (key == GLFW_KEY_SPACE) debug_display = !debug_display;
    }

    if(action == GLFW_RELEASE) {
        key_state[key] = false;
    }
}

void input(GLFWwindow* window, float* player_x, float* player_y, float dt) {
    // Iterate over all keys to see which are held or not
    for (int key = 0; key < NUM_KEYS; key++) {
        if (key_state[key] == false) continue;
        switch (key) {
            case GLFW_KEY_ESCAPE: 
                glfwSetWindowShouldClose(window, GLFW_TRUE);
                break;
            case GLFW_KEY_W:
                *player_y = *player_y - 1.0f * dt;
                break;
            case GLFW_KEY_A:
                *player_x = *player_x - 1.0f * dt;
                break;
            case GLFW_KEY_S:
                *player_y = *player_y + 1.0f * dt;
                break;
            case GLFW_KEY_D:
                *player_x = *player_x + 1.0f * dt;
                break;
        }
    }
}


GLuint load_shader(std::string vertex_name, std::string fragment_name) {
    GLuint shader_program;
    std::string vertexShaderSource;
    std::string fragmentShaderSource;

    // Generate the shader program
    vertexShaderSource = get_shader_src(vertex_name);
    fragmentShaderSource = get_shader_src(fragment_name);

    if (vertexShaderSource.empty() || vertexShaderSource.data() == nullptr) {
        std::cout << "Failed to locate the vertex shader!";
        return -1;
    }
    if (fragmentShaderSource.empty() || fragmentShaderSource.data() == nullptr){
        std::cout << "Failed to locate the fragment shader!";
        return -1;
    }

    // Compile shaders with error checking
    GLuint vertexShader = compile_shader(vertexShaderSource.c_str(), GL_VERTEX_SHADER);
    GLuint fragmentShader = compile_shader(fragmentShaderSource.c_str(), GL_FRAGMENT_SHADER);
    //
    if (!vertexShader || !fragmentShader) {
        fprintf(stderr, "Shader compilation failed\n");
        return -1;
    }

    shader_program = glCreateProgram();
    glAttachShader(shader_program, vertexShader);
    glAttachShader(shader_program, fragmentShader);
    glLinkProgram(shader_program);

    // Check linking
    GLint success;
    glGetProgramiv(shader_program, GL_LINK_STATUS, &success);
    if (!success) {
        char infoLog[4096];
        glGetProgramInfoLog(shader_program, 4096, NULL, infoLog);
        fprintf(stderr, "Shader linking failed: %s\n", infoLog);
        return -1;
    }

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    return shader_program;
}


int main(){
    std::cout << "entered!" << std::endl;
    GLFWwindow* window;

    // Stores pixel data to be rendered to the texture.
    GLuint pixel_vbo;
    GLuint texture;
    GLuint vao;
    GLuint vertex_vbo;
    // Stores the quad vertices that we will render textures to.
    GLuint instance_vbo;


    float player_x = 0;
    float player_y = 0;

    // ==================== Initialise window ======================

    if (!glfwInit()) {
        std::cout << "Error initialising glfw!" << std::endl;
        return -1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWmonitor* monitor = glfwGetPrimaryMonitor();
    const GLFWvidmode* mode = glfwGetVideoMode(monitor);

    // Create fullscreen window
    window = glfwCreateWindow(mode->width, mode->height, "Sand Simulation", monitor, NULL);

    if (!window) {
        std::cout << "Failed to create the window!" << std::endl;
        return -1;
    }
    glfwMakeContextCurrent(window);

    std::cout << "Readying glew!" << std::endl;

    glewExperimental = GL_TRUE;
    GLenum glewStatus = glewInit();
    if (glewStatus != GLEW_OK) {
        std::cerr << "glewInit failed: " << glewGetErrorString(glewStatus) << std::endl;
        return -1;
    }
    // In core profile, glewInit often leaves a spurious GL_INVALID_ENUM from glGetStringi.
    // Consume it so it doesn't break later error checks.
    glGetError();

    // Verify the specific functions you're about to call actually loaded:
    if (!glGenBuffers || !glBindBuffer || !glBufferData) {
        std::cerr << "Critical GL functions missing — GLEW did not load them." << std::endl;
        return -1;
    }

    std::cout << "glewInit() succeeded" << std::endl;

    // =========================== Initialise buffers ===============

    for (int material_code = 0; material_code < 256; material_code++) {
        material_data[material_code] = {0};
    }
    load_materials(colours,material_data);

    // Create a VBO to hold our pixel data 
    glGenBuffers(1, &pixel_vbo);
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, pixel_vbo);
    glBufferData(GL_PIXEL_UNPACK_BUFFER, WORLD_WIDTH * WORLD_HEIGHT * sizeof(char), 
            NULL, GL_DYNAMIC_DRAW);
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);

    // Create a texture to display
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, WORLD_WIDTH, WORLD_HEIGHT, 0, 
            GL_RED, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glBindTexture(GL_TEXTURE_2D, 0);

    GLuint instance_shader_program = load_shader("instance.vert", "instance.frag");

    glUseProgram(instance_shader_program);

    // Sends the material - colour data from materials.json to the shader
    GLint colours_loc = glGetUniformLocation(instance_shader_program, "colours");
    if (colours_loc == -1) {
        fprintf(stderr, "Warning: 'colours' uniform not found in shader\n");
    } else {
        glUniform1fv(colours_loc, 256 * 3, colours);
    } 
    // Sends the world dimensions to the shader    
    GLint world_size_loc = glGetUniformLocation(instance_shader_program, "uWorldSize");
    if (world_size_loc == -1) {
        fprintf(stderr, "Warning: 'world_size' uniform not found in shader\n");
    }
    float unit_quad[] = {
        // positions   // texture coords
        -0.5f,  0.5f,  0.0f, 1.0f,  // top-left
        -0.5f, -0.5f,  0.0f, 0.0f,  // bottom-left
         0.5f, -0.5f,  1.0f, 0.0f,   // bottom-right
         0.5f,  0.5f,  1.0f, 1.0f    // top-right
    };


    glGenBuffers(1, &vertex_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vertex_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(unit_quad), unit_quad, GL_STATIC_DRAW);

    // Create VAO and VBO for the objects
    glGenBuffers(1, &instance_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, instance_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(BodyInstance) * MAX_OBJECTS, nullptr, GL_DYNAMIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    // --- per-vertex: the unit quad ---
    glBindBuffer(GL_ARRAY_BUFFER, vertex_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(unit_quad), unit_quad, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4*sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4*sizeof(float), (void*)(2*sizeof(float)));
    glEnableVertexAttribArray(1);

    // --- per-instance: the instance buffer ---
    glBindBuffer(GL_ARRAY_BUFFER, instance_vbo);
    const GLsizei stride = sizeof(BodyInstance);

    // Add x,y
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, stride, (void*)offsetof(BodyInstance, x_pos));
    glEnableVertexAttribArray(2);
    glVertexAttribDivisor(2, 1);

    // Add sx,sy
    glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, stride, (void*)offsetof(BodyInstance, x_scale));
    glEnableVertexAttribArray(3);
    glVertexAttribDivisor(3, 1);

    // Add rotation
    glVertexAttribPointer(4, 1, GL_FLOAT, GL_FALSE, stride, (void*)offsetof(BodyInstance, angle));
    glEnableVertexAttribArray(4);
    glVertexAttribDivisor(4, 1);

    // Add rgb colour
    glVertexAttribPointer(5, 3, GL_FLOAT, GL_FALSE, stride, (void*)offsetof(BodyInstance, r));
    glEnableVertexAttribArray(5);
    glVertexAttribDivisor(5, 1);

    glBindVertexArray(0);

    // Position attribute
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // Texture coord attribute
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);  // Unbind

    glfwSetKeyCallback(window, key_callback);
    // =============== Engine setup ============================

    // Create the world as 1 byte per pixel
    char* render_buffer = new char[WORLD_WIDTH * WORLD_HEIGHT];
    // May need to be every pixel or every cell, cell = more complex as alternating each frame
    char* dirty = new char[WORLD_WIDTH * WORLD_HEIGHT];
    DataPoint* data_buffer = init_data_buffer(WORLD_WIDTH,WORLD_HEIGHT);

    for (int y = 0; y < WORLD_HEIGHT; y++) {
        for (int x = 0; x < WORLD_WIDTH; x++) {
            render_buffer[y * WORLD_WIDTH + x] = ' ';
            // Defaults to displaying air, aka 32 ticks of dirty time.
            dirty[y * WORLD_WIDTH + x] = ' ';
            data_buffer[y * WORLD_WIDTH + x].material = ' ';
            if (x == 0 || x == WORLD_WIDTH - 1 || y == 0 || y == WORLD_HEIGHT - 1) {
                data_buffer[y * WORLD_WIDTH + x].material = 'R';
            }
        }
    }

    // =============== Main Loop ===============================
    unsigned long step = 0;

    int NUM_UPDATES = 1;

    srand (static_cast <unsigned> (time(0)));

    // const auto FRAME_DURATION = duration<double>(1.0 / TARGET_FPS);
    const std::chrono::steady_clock::duration FRAME_DURATION = 
        std::chrono::duration_cast<std::chrono::steady_clock::duration>(
                std::chrono::duration<double>(1.0 / TARGET_FPS)
                );
    std::chrono::steady_clock::time_point frame_start = std::chrono::steady_clock::now();

    auto worldId = box2d_init();

    while (!glfwWindowShouldClose(window)) {

        box2d_update(worldId);

        frame_start = std::chrono::steady_clock::now();

        // Clear screen
        glClear(GL_COLOR_BUFFER_BIT);

        // Enable the GL shader and draw the current frame data to the screen
        // glUseProgram(shader_program);
        // glBindVertexArray(vao);
        // glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

        glBindBuffer(GL_ARRAY_BUFFER, instance_vbo);

        box2d_bind();

        glBindBuffer(GL_ARRAY_BUFFER, 0);

        glUseProgram(instance_shader_program);
        glUniform2f(world_size_loc, WORLD_WIDTH, WORLD_HEIGHT);
        glBindTexture(GL_TEXTURE_2D, texture);
        glBindVertexArray(vao);
        box2d_draw();
        glBindVertexArray(0);

        // Update the screen and check for user input
        glfwSwapBuffers(window);
        glfwPollEvents();
        input(window,&player_x,&player_y,1.0f);

        // Use the data_buffer to compute the next gamestate, and update what the render_buffer sees.
        render_buffer = update_step(render_buffer,data_buffer,dirty,step,NUM_UPDATES,player_x,player_y);

        step+=NUM_UPDATES;

        // Update the texture to store the render_buffer's data.
        glBindTexture(GL_TEXTURE_2D, texture);
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, WORLD_WIDTH, WORLD_HEIGHT, 
                GL_RED, GL_UNSIGNED_BYTE, render_buffer);
        glBindTexture(GL_TEXTURE_2D, 0);


        auto frame_end = std::chrono::steady_clock::now();
        auto elapsed = frame_end - frame_start;

        if (elapsed < FRAME_DURATION) {
            auto sleep_time = FRAME_DURATION - elapsed;
            // Sleep n microseconds
            // Use the custom sleep method that picks 
            // windows / linux's sleep function
            u_sleep(sleep_time.count() / 1000);
        }
    }
    // =============== Cleanup =================================

    std::cout << "Cleanup" << std::endl;

    b2DestroyWorld(worldId);
    // Clean up OpenGL resources
    glDeleteBuffers(1, &pixel_vbo);
    glDeleteBuffers(1, &vertex_vbo);
    glDeleteTextures(1, &texture);
    glDeleteVertexArrays(1, &vao);
    glDeleteProgram(instance_shader_program);

    // Destroy window
    if (window) {
        glfwDestroyWindow(window);
        window = nullptr;
    }

    // Terminate GLFW
    glfwTerminate();

    delete[] colours;
    delete[] material_data;

    delete[] render_buffer;
    delete[] data_buffer;

    std::cout << "Exit" << std::endl;


    return 0;
}
