#include "app.h"

// Order is important for these 2!
#include "GL/glew.h"
#include "GLFW/glfw3.h"

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
        return 0;
    }
    return shader;
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

    // =========================== Initialise buffers ===============



    // // Create a VBO to hold our pixel data (for PBO async transfer - optional)
    // glGenBuffers(1, &pixel_vbo);
    // glBindBuffer(GL_PIXEL_UNPACK_BUFFER, pixel_vbo);
    // glBufferData(GL_PIXEL_UNPACK_BUFFER, WORLD_WIDTH * WORLD_HEIGHT * sizeof(char), 
    //         NULL, GL_DYNAMIC_DRAW);
    // glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);  // Unbind for now
    //
    // // Create a texture to display
    // glGenTextures(1, &texture);
    // glBindTexture(GL_TEXTURE_2D, texture);
    // glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, WORLD_WIDTH, WORLD_HEIGHT, 0, 
    //         GL_RED, GL_UNSIGNED_BYTE, NULL);
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    // glBindTexture(GL_TEXTURE_2D, 0);

    vertexShaderSource = get_shader_src("vertex.vert");
    fragmentShaderSource = get_shader_src("fragment.frag");
    // Compile shaders with error checking
    // GLuint vertexShader = compile_shader(vertexShaderSource.c_str(), GL_VERTEX_SHADER);
    // GLuint fragmentShader = compile_shader(fragmentShaderSource.c_str(), GL_FRAGMENT_SHADER);
    //
    // if (!vertexShader || !fragmentShader) {
    //     fprintf(stderr, "Shader compilation failed\n");
    //     return -1;
    // }

    // shader_program = glCreateProgram();
    // glAttachShader(shader_program, vertexShader);
    // glAttachShader(shader_program, fragmentShader);
    // glLinkProgram(shader_program);
    //
    // // Check linking
    // GLint success;
    // glGetProgramiv(shader_program, GL_LINK_STATUS, &success);
    // if (!success) {
    //     char infoLog[4096];
    //     glGetProgramInfoLog(shader_program, 4096, NULL, infoLog);
    //     fprintf(stderr, "Shader linking failed: %s\n", infoLog);
    //     return -1;
    // }

    // glDeleteShader(vertexShader);
    // glDeleteShader(fragmentShader);

    // Magenta = undefined material colour

    // glUseProgram(shader_program);

    // GLint coloursLoc = glGetUniformLocation(shader_program, "colours");
    // if (coloursLoc == -1) {
    //     fprintf(stderr, "Warning: 'colours' uniform not found in shader\n");
    // } else {
    //     glUniform1fv(coloursLoc, 256 * 3, colours);
    // } 
    // float vertices[] = {
    //     // positions   // texture coords
    //     -1.0f,  1.0f,  0.0f, 0.0f,  // top-left
    //     -1.0f, -1.0f,  0.0f, 1.0f,  // bottom-left
    //     1.0f, -1.0f,  1.0f, 1.0f,  // bottom-right
    //     1.0f,  1.0f,  1.0f, 0.0f   // top-right
    // };
    //
    // // Create VAO and VBO for the quad (different VBO!)
    // glGenVertexArrays(1, &vao);
    // glGenBuffers(1, &vertex_vbo);  // Different from pixel_vbo
    // glBindVertexArray(vao);
    // glBindBuffer(GL_ARRAY_BUFFER, vertex_vbo);
    // glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    //
    // // Position attribute
    // glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    // glEnableVertexAttribArray(0);
    //
    // // Texture coord attribute
    // glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    // glEnableVertexAttribArray(1);
    //
    // glBindVertexArray(0);  // Unbind

    std::cout << "Exit" << std::endl;
    return 0;
}
