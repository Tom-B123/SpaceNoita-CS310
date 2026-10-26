#ifndef UTIL_H
#define UTIL_H
#include<stdlib.h>
#include<fstream>
#include<vector>

#include "GL/glew.h"
#include "GLFW/glfw3.h"

/**
 * Locate a given file
 */
std::string find_shader_file(std::string shader);
/**
 * Converts a file location into a string containing that files contents.
 */
std::string get_shader_src(std::string shader);

/**
 *  Compilers the given shader source code.
 */
GLuint compile_shader(const char* source, GLenum type);

enum SandState {
 SOLID  = 1,
 LIQUID = 2,
 POWDER = 3,
 GAS = 4,
 INVALID = -1
};

/**
 *  Convert the state into a SandState. e.g. "Solid" -> SOLID
 */
SandState state_from_string(std::string state_string);


/** 
 * Converts a #RRGGBB colour to a numerical value
 * @param colour the hex code
 * @param offset the hex code
 */
int hex2(char colour[8],int offset);

#endif
