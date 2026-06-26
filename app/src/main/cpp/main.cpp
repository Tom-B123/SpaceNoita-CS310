#include "app.h"
#include "graphics.h"
#include "cl_setup.h"
#include "engine.h"



int main(){
    GameWindow window(WORLD_WIDTH,WORLD_HEIGHT);

    while (window.is_open()) {
        window.refresh();
    }
    Sleep(100);
    Sleep(100);
}
