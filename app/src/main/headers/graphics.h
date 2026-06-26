#ifndef GRAPHICS_H 
#define GRAPHICS_H 

#include "GL/glew.h"
#include "GLFW/glfw3.h"
// #include "GLFW/glfw3native.h"
#include "app.h"
#include "cl_setup.h"
// Setup OpenGL and the window
// Link with OpenCL to connect the processing buffer to the window display buffer
class GameWindow {
    private:
        GLFWwindow* window;
        GLuint pixel_vbo;      // Renamed for clarity
        GLuint texture;
        GLuint vao;
        GLuint vertex_vbo;     // Separate VBO for vertices
        GLuint shader_program;
        int width;
        int height;
        double pixel_scale;
        float* colours; 
        std::string vertexShaderSource;
        std::string fragmentShaderSource;
    public:
        bool success;
        GameWindow(int n_width, int n_height);
        ~GameWindow() {
            std::cout << "destructor" << std::endl;
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
        }
        void set_colour(char material, float r, float g, float b); 
        GLuint compile_shader(const char* source, GLenum type);
        void update_texture(char* data);
        void close();
        void draw() ;
        void refresh();
        bool is_open();
        GLFWwindow* get_window();
        void hide();
};

void print_buf(int width, int height, char* buf);
void draw(int width, int height, char* data);

#endif
