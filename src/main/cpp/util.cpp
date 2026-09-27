#include"util.h"

#include <iostream>
// Locate a given file
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

// Converts a file location into a string containing that files contents.
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


// Get the state integer / enum from a string
int state_from_string(std::string state_string) {
        if (!state_string.compare("powder"))      { return POWDER; }
        else if (!state_string.compare("liquid")) { return LIQUID; }
        else if (!state_string.compare("gas"))    { return GAS; }
        else if (!state_string.compare("solid"))  { return SOLID; }
        return -1;
}

// Converts a #RRGGBB colour to a numerical value
int hex2(char colour[8],int offset) {
    int total = 0;

    // Convert hex digits to n + 10
    if (colour[offset+1] >= 'A' && colour[offset+1] <='F') total += colour[offset+1] - 'A' + 10;
    // Convert numerical digits to n
    else if (colour[offset+1] >= '0' && colour[offset+1] <='9') total += (colour[offset+1]-'0');
    else return -1;
    // Convert hex digits to 16(n + 10)
    if (colour[offset] >= 'A' && colour[offset] <='F') total += 16 * (colour[offset] - 'A' + 10);
    // Convert numerical digits to 16(n)
    else if (colour[offset] >= '0' && colour[offset] <='9') total += 16 * (colour[offset]-'0');
    else return -1;

    // return total if it is valid hex code, else -1
    return total;
}
