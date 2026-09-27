#include "materials.h"
#include <algorithm>
#include <bitset>
#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include "app.h"

#include <cstdlib>
// Add this before including RapidJSON

#include <fstream>
#include <iostream>

#include"util.h"
#include"engine.h"
int sand_count;

Material* material_data = new Material[256];
float* colours = new float[256 * 3];
// Each rule is one byte, storing the resulting state in its corresponding initial state position
// Lookup table stores 16 rules for stable/neutral rules for left and for  right configurations, 
// but only 16 total will be expected.
// Each state has unique rules.
// char rules[NUM_STATES * 3 * 16] = {0};


bool SHOW_MATERIAL_COUNTS = false;
bool SHOW_MATERIAL_CHANGES = false;
bool SHOW_MATERIAL_COLOURS = false;
bool SHOW_RULES_DEBUG = false;
bool SHOW_RULES = false;
bool SHOW_BITSETS = false;
bool SHOW_REORDERING = false;

bool SPAWN_SAND = true;





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



char* update_step(char* render_buffer,DataPoint* data_buffer,long step,
                    int num_updates) {

    for (int i = 0; i < num_updates; i++) {
        if (SPAWN_SAND && (i + step) % 3 == 0) {
            spawn_material(20,0,'W',data_buffer);
            spawn_material(3,0,'S',data_buffer);
            spawn_material(WORLD_WIDTH - 3,0,'I',data_buffer);
            spawn_material(1,8,'R',data_buffer);
            spawn_material(2,8,'R',data_buffer);
            spawn_material(3,8,'R',data_buffer);
            spawn_material(4,8,'R',data_buffer);
            spawn_material(8,8,'R',data_buffer);
        }
        // Update using margolus neighbourhood.
        for (int y = 0; y < WORLD_HEIGHT / 2; y++) {
            for (int x = 0; x < WORLD_WIDTH / 2; x++) {
                // data_buffer = simple_sand_update(data_buffer,step + i,x,y);
                // data_buffer = bitmap_update(data_buffer,step + i,x,y);
                data_buffer = simple_margolus_update(material_data,data_buffer,step + i,x,y);
            }
        }
    }

    int cur_count = 0;
    for (int y = 0; y < WORLD_HEIGHT; y++) {
        for (int x = 0; x < WORLD_WIDTH; x++) {
            if (data_buffer[y*WORLD_WIDTH + x].material == 'S') cur_count++;
            render_buffer[y*WORLD_WIDTH + x] = data_buffer[y * WORLD_WIDTH + x].material;
        }
    }
    if (SHOW_MATERIAL_COUNTS) std::cout << cur_count << ", expecting: " << sand_count << std::endl;

    return render_buffer;
}


int main(){
    GLFWwindow* window;
    std::string vertexShaderSource;
    std::string fragmentShaderSource;


    GLuint pixel_vbo;
    GLuint texture;
    GLuint vao;
    GLuint vertex_vbo;
    GLuint shader_program;


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

    glewExperimental = GL_TRUE;

    glewInit();

    // =========================== Initialise buffers ===============

    for (int material_code = 0; material_code < 256; material_code++) {
        material_data[material_code] = {0};
    }
    load_materials(colours,material_data);

    for (int material_code = 0; material_code < 256; material_code++) {
        Material material = material_data[material_code];
        // std::cout << material.name << ": " << material.colour << std::endl;
    }

    // Create a VBO to hold our pixel data (for PBO async transfer - optional)
    glGenBuffers(1, &pixel_vbo);
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, pixel_vbo);
    glBufferData(GL_PIXEL_UNPACK_BUFFER, WORLD_WIDTH * WORLD_HEIGHT * sizeof(char), 
            NULL, GL_DYNAMIC_DRAW);
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);  // Unbind for now

    // Create a texture to display
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, WORLD_WIDTH, WORLD_HEIGHT, 0, 
            GL_RED, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glBindTexture(GL_TEXTURE_2D, 0);

    vertexShaderSource = get_shader_src("vertex.vert");
    fragmentShaderSource = get_shader_src("fragment.frag");

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

    // Magenta = undefined material colour

    glUseProgram(shader_program);

    GLint coloursLoc = glGetUniformLocation(shader_program, "colours");
    if (coloursLoc == -1) {
        fprintf(stderr, "Warning: 'colours' uniform not found in shader\n");
    } else {
        glUniform1fv(coloursLoc, 256 * 3, colours);
    } 
    float vertices[] = {
        // positions   // texture coords
        -1.0f,  1.0f,  0.0f, 0.0f,  // top-left
        -1.0f, -1.0f,  0.0f, 1.0f,  // bottom-left
        1.0f, -1.0f,  1.0f, 1.0f,  // bottom-right
        1.0f,  1.0f,  1.0f, 0.0f   // top-right
    };

    // Create VAO and VBO for the quad (different VBO!)
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vertex_vbo);  // Different from pixel_vbo
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vertex_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // Position attribute
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // Texture coord attribute
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);  // Unbind

    // =============== Engine setup ============================

    // Create the world as 1 byte per pixel
    char* render_buffer = new char[WORLD_WIDTH * WORLD_HEIGHT];
    DataPoint* data_buffer = init_data_buffer(WORLD_WIDTH,WORLD_HEIGHT);

    char choices[5] = {' ',' ',' ',' ',' '};

    for (int y = 0; y < WORLD_HEIGHT; y++) {
        for (int x = 0; x < WORLD_WIDTH; x++) {
            char material = choices[(y * WORLD_WIDTH + x) % 5];
            render_buffer[y * WORLD_WIDTH + x] = material;
            data_buffer[y * WORLD_WIDTH + x].material = material;
        }
    }

    // =============== Main Loop ===============================
    unsigned long step = 0;

    int NUM_UPDATES = 10;

    while (!glfwWindowShouldClose(window)) {
        // Sleep(10);
        glClear(GL_COLOR_BUFFER_BIT);
        glUseProgram(shader_program);
        glBindTexture(GL_TEXTURE_2D, texture);
        glBindVertexArray(vao);
        glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

        glfwSwapBuffers(window);
        glfwPollEvents();
        if (glfwGetKey(window,GLFW_KEY_ESCAPE) == GLFW_PRESS) {
            glfwSetWindowShouldClose(window, GLFW_TRUE);
        }
        
        usleep(30000);
        render_buffer = update_step(render_buffer,data_buffer,step,NUM_UPDATES);

        step+=NUM_UPDATES;

        glBindTexture(GL_TEXTURE_2D, texture);
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, WORLD_WIDTH, WORLD_HEIGHT, 
                GL_RED, GL_UNSIGNED_BYTE, render_buffer);
        glBindTexture(GL_TEXTURE_2D, 0);
    }
    // =============== Cleanup =================================

    std::cout << "Cleanup" << std::endl;
    // Clean up OpenGL resources
    glDeleteBuffers(1, &pixel_vbo);
    glDeleteBuffers(1, &vertex_vbo);
    glDeleteTextures(1, &texture);
    glDeleteVertexArrays(1, &vao);
    glDeleteProgram(shader_program);

    // Destroy window
    if (window) {
        glfwDestroyWindow(window);
        window = nullptr;
    }

    // Terminate GLFW
    glfwTerminate();
    
    free(colours);
    free(material_data);
    std::cout << "Exit" << std::endl;


    return 0;
}
