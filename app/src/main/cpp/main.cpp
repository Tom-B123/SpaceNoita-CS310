#include "app.h"
#include "graphics.h"
#include "cl_setup.h"
#include "engine.h"
#include "buffer.h"



int main(){

    GameWindow window(WORLD_WIDTH,WORLD_HEIGHT,PIXEL_SCALE);

    buffer buf = init_buf(WORLD_WIDTH, WORLD_HEIGHT,' ');

    CL cl(buf);

    Engine engine(WORLD_WIDTH, WORLD_HEIGHT, &cl,&window);
    
    int error = engine.main_loop();

    // return error;
    //
    return 0;
}
