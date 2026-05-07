#include "app.h"
#include "graphics.h"


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
