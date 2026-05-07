#ifndef GRAPHICS_H 
#define GRAPHICS_H 
#include "app.h"
// Setup OpenGL and the window
// Link with OpenCL to connect the processing buffer to the window display buffer
class Window {
    private:
        GLFWwindow* window;
    public:
        Window() {
    
            /* Initialize the library */
            if (!glfwInit())
                return;

            /* Create a windowed mode window and its OpenGL context */
            window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Hello World", NULL, NULL);
            if (!window)
            {
                glfwTerminate();
                return;
            }

            /* Make the window's context current */
            glfwMakeContextCurrent(window);
        }
        void close() {
            glfwTerminate();
        }
};

void print_buf(int width, int height, char* buf);
void draw(int width, int height, char* data);

#endif
