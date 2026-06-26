#include "graphics.h"
#include "GLFW/glfw3.h"


void print_buf(int width, int height, char* buf) {
    if (width%2 == 1) width++;
    if (height%2 == 1) height++;
    std::cout << "\033[H";

    std::cout.write(buf,(width+3) * (height+2));
    std::cout << std::endl;
    
}

void draw(int width, int height, char* data) {
    print_buf(width,height,data);
}

GameWindow::GameWindow(int n_width, int n_height) : 
    width(n_width), height(n_height), window(nullptr)
    {
    if (!glfwInit()) {
        std::cout << "Error initialising glfw!" << std::endl;
        return;
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
        return;
    }
    glfwMakeContextCurrent(window);

    glewExperimental = GL_TRUE;

    // Create a VBO to hold our pixel data (for PBO async transfer - optional)
    glGenBuffers(1, &pixel_vbo);
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, pixel_vbo);
    glBufferData(GL_PIXEL_UNPACK_BUFFER, width * height * sizeof(char), 
            NULL, GL_DYNAMIC_DRAW);
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);  // Unbind for now

    // Create a texture to display
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, width, height, 0, 
            GL_RED, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glBindTexture(GL_TEXTURE_2D, 0);

    vertexShaderSource = get_shader_src("vertex.vert");
    fragmentShaderSource = get_shader_src("fragment.frag");
    // Compile shaders with error checking
    GLuint vertexShader = compile_shader(vertexShaderSource.c_str(), GL_VERTEX_SHADER);
    GLuint fragmentShader = compile_shader(fragmentShaderSource.c_str(), GL_FRAGMENT_SHADER);

    if (!vertexShader || !fragmentShader) {
        fprintf(stderr, "Shader compilation failed\n");
        return;
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
        return;
    }

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    // Magenta = undefined material colour
    for (int i = 0; i < 256; i++) {
        set_colour(i,1.0,0.0,1.0);
    }

    // Define materials here
    set_colour(' ',0.2,0.2,0.2);
    set_colour('S',1.0,1.0,0.0);
    set_colour('W',0.0,0.0,1.0);
    set_colour('O',0.0,0.0,0.0);
    set_colour('R',0.5,0.5,0.5);

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
    

}

void GameWindow::set_colour(char material, float r, float g, float b) {
    int index = (int)material;
    colours[3 * index + 0] = r;
    colours[3 * index + 1] = g;
    colours[3 * index + 2] = b;
}

GLuint GameWindow::compile_shader(const char* source, GLenum type) {
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, NULL);
    glCompileShader(shader);

    GLint success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetShaderInfoLog(shader, 512, NULL, infoLog);
        fprintf(stderr, "Shader compilation failed: %s\n", infoLog);
        return 0;
    }
    return shader;
}

void GameWindow::update_texture(char* data) {
    // Upload simulation data to texture
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height, 
            GL_RED, GL_UNSIGNED_BYTE, data);
    glBindTexture(GL_TEXTURE_2D, 0);
}

void GameWindow::close() {
    glfwSetWindowShouldClose(window, GLFW_TRUE);
}

void GameWindow::draw() {
    glClear(GL_COLOR_BUFFER_BIT);
    glUseProgram(shader_program);
    glBindTexture(GL_TEXTURE_2D, texture);
    glBindVertexArray(vao);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

    // Don't unbind here - let refresh handle buffer swap
}

void GameWindow::refresh() {
    glfwSwapBuffers(window);
    glfwPollEvents();
}

bool GameWindow::is_open() {
    if (!window) {
        std::cout << "No window, crashing!" << std::endl;
        return false;
    }
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        close();
    }
    return !glfwWindowShouldClose(window);
}

GLFWwindow* GameWindow::get_window() {
    return window;
}

void GameWindow::hide() {
}
