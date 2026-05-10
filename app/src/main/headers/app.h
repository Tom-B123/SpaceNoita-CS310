#ifndef APP_H
#define APP_H

#include "CL/cl.h"
#include "CL/opencl.hpp"
#include <chrono>
#include <iomanip>
#include <iostream>
#include <stdlib.h>
#include <thread>
#include <vector>
#include <fstream>
#include <conio.h>
#include <stdio.h>
#include <Windows.h>
#include <string>

const int WORLD_WIDTH = 1920;
const int WORLD_HEIGHT = 1080;
const double PIXEL_SCALE = 1;
const int CELL_WIDTH = 2;
const int CELL_HEIGHT = 2;

class FPSCount {
    private:
        std::chrono::steady_clock::time_point last_time;
        double target_fps;
    public:
        FPSCount(double n_target_fps) : target_fps(n_target_fps){
            last_time = std::chrono::steady_clock::now();
        }
        void nextFrame() {
            auto new_time = std::chrono::steady_clock::now();
            
            double delta = std::chrono::duration_cast<std::chrono::milliseconds>(new_time - last_time).count();
            // std::cout << "\033[1;0HFPS: " << std::fixed << std::setprecision(1) << (1000.0/delta) << "     ";
            std::cout.flush();

            double expected_time = 1000.0 / target_fps;

            std::this_thread::sleep_for(std::chrono::milliseconds((long)(expected_time - delta)));
            last_time = new_time;
        }
};

#endif
