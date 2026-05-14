#include "app.h"
#include "graphics.h"
#include "cl_setup.h"
#include "engine.h"



int main(){
    int error;
    {   
        GameWindow window(WORLD_WIDTH,WORLD_HEIGHT);
        {
            CL cl = CL();

            Engine engine(WORLD_WIDTH, WORLD_HEIGHT, &cl,&window);

            glfwSetWindowUserPointer(window.get_window(), &engine);
            glfwSetKeyCallback(window.get_window(), Engine::keyCallback);

            error = engine.main_loop();
        }
    }
    
    std::cout << "Exiting with error: " << error << std::endl;

    std::cout.flush();

    TerminateProcess(GetCurrentProcess(),error);

    std::cout << "Forced exit: " << error << std::endl;
    
    _exit(error);
}
