#define NOMINMAX
#define WIN32_LEAN_AND_MEAN

#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/filereadstream.h"

#include "app.h"

// Order is important for these 2!
#include "GL/glew.h"
#include "GLFW/glfw3.h"
#include <cstdlib>
// Add this before including RapidJSON

#include <fstream>
#include <iostream>

std::string find_shader_file(std::string shader) {
    std::vector<std::string> search_paths = {
        "app/src/main/shaders/" + shader,
        "../app/src/main/shaders/" + shader,
        "../../app/src/main/shaders/" + shader,
        "../../../app/src/main/shaders/" + shader,
        "src/main/shaders/" + shader,
        "../src/main/shaders/" + shader,
        "" + shader,
        "./" + shader
    };

    for (const auto& path : search_paths) {
        std::ifstream test(path);
        if (test.is_open()) {
            test.close();
            return path;
        }
    }

    return "C:/Users/tomhb/University/cs310/FallingSandIter1/app/src/main/shaders/" + shader;
}

std::string get_shader_src(std::string shader) {
    std::ifstream hello_world_file(find_shader_file(shader));
    std::string src(std::istreambuf_iterator<char>(hello_world_file), (std::istreambuf_iterator<char>()));
    return src;
}


GLuint compile_shader(const char* source, GLenum type) {
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, NULL);
    glCompileShader(shader);

    GLint success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetShaderInfoLog(shader, 512, NULL, infoLog);
        fprintf(stderr, "Shader compilation failed: %s\n", infoLog);
        switch (type) {
            case GL_VERTEX_SHADER: 
                std::cout << "Error in Vertex shader!";
                break;
            case GL_FRAGMENT_SHADER: 
                std::cout << "Error in Fragment shader!";
                break;
        }
        std::cout << std::endl;
        return 0;
    }
    return shader;
}

struct DataPoint {
    char material;
    bool updated;
};

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

char* update_step(char* render_buffer,DataPoint* data_buffer) {


    int counts[256] = {0};
    for (int y = 0; y < WORLD_HEIGHT; y++) {
        for (int x = 0; x < WORLD_WIDTH; x++) {
            data_buffer[y*WORLD_WIDTH + x].updated = false;
            counts[(int)data_buffer[y*WORLD_WIDTH + x].material]++;
        }
    }
    for (int y = 0; y < WORLD_HEIGHT; y++) {
        for (int x = 0; x < WORLD_WIDTH; x++) {
            DataPoint val = data_buffer[y*WORLD_WIDTH + x];
            if (y < WORLD_HEIGHT - 1) {
                if (val.material == 'S') {
                    DataPoint oth = data_buffer[(y+1)*WORLD_WIDTH + x];
                    if (!val.updated && !oth.updated && oth.material != 'S') {
                        oth.updated = true;
                        val.updated = true;
                        data_buffer[(y)*WORLD_WIDTH + x] = oth;
                        data_buffer[(1+y)*WORLD_WIDTH + x] = val;
                    }
                }
            }

        }   
    }
    for (int y = 0; y < WORLD_HEIGHT; y++) {
        for (int x = 0; x < WORLD_WIDTH; x++) {
            render_buffer[y*WORLD_WIDTH + x] = data_buffer[y * WORLD_WIDTH + x].material;
        }
    }
    for (int i = 0; i < 256; i++) {
        if (counts[i] > 0) {
            std::cout << (char)i << ": " << counts[i] << std::endl;
        }
    }


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

    float* colours = new float[256 * 3];


    // Read the entire file into a string
    std::string location = find_shader_file("../cpp/materials.json");

    std::cout << location << std::endl;

    FILE* fp = fopen(location.c_str(), "r");

    // Use a FileReadStream to
      // read the data from the file
    char readBuffer[65536];
    rapidjson::FileReadStream is(fp, readBuffer,
                                 sizeof(readBuffer));

    // Parse the JSON data 
      // using a Document object
    rapidjson::Document d;
    d.ParseStream(is);

    // Close the file
    fclose(fp);

    rapidjson::Value& sand = d["S"];
    std::cout << sand.FindMember("state")->value.GetString() << std::endl;
    // std::cout << sand["density"].GetInt() << std::endl;
    // Access the data in the JSON document

    for (int material = 0; material < 256; material++) {
        int r;
        int g;
        int b;

        switch (material) {
            case 'S':
                r=255;g=255;b=0;break;
            case 'W':
                r=0;g=0;b=255;break;
            case 's':
                r=200;g=200;b=200;break;
            case 'O':
                r=128;g=0;b=0;break;
        }
        colours[3 * material + 0] = r;
        colours[3 * material + 1] = g;
        colours[3 * material + 2] = b;
    }

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

    char choices[4] = {'S','s','W','O'};

    for (int y = 0; y < WORLD_HEIGHT; y++) {
        for (int x = 0; x < WORLD_WIDTH; x++) {
            char material = choices[std::rand() & 0b11];
            render_buffer[y * WORLD_WIDTH + x] = material;
            data_buffer[y * WORLD_WIDTH + x].material = material;
        }
    }

    // =============== Main Loop ===============================

    while (!glfwWindowShouldClose(window)) {
        Sleep(500);
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

        render_buffer = update_step(render_buffer,data_buffer);

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

    std::cout << "Exit" << std::endl;

    return 0;
}
