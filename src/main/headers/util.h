
#include<stdlib.h>
#include<fstream>
#include<vector>

#include "GL/glew.h"
#include "GLFW/glfw3.h"

// Locate a given file
std::string find_shader_file(std::string shader);
// Converts a file location into a string containing that files contents.
std::string get_shader_src(std::string shader);

GLuint compile_shader(const char* source, GLenum type);


#define SOLID 1
#define LIQUID 2
#define POWDER 3
#define GAS 4

int state_from_string(std::string state_string);


// Converts a #RRGGBB colour to a numerical value
int hex2(char colour[8],int offset);
