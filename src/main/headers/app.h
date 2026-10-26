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
// #include <conio.h>
#include <stdio.h>
#include <cmath>
#include"materials.h"

#include "box2d/math_functions.h"
#include "box2d/box2d.h"

#ifdef WIN32
#include <Windows.h>
#endif

#ifdef __linux__
#include<unistd.h>
#endif

inline void u_sleep(long micros) {
#ifdef WIN32
    Sleep(static_cast<DWORD>(micros / 1000));
#elif defined(__linux__)
    usleep(static_cast<useconds_t>(micros));
#endif
}

#include <string>

// Get random float from range
float rand_range(float start, float end);

/**
 * The amount of ticks it takes for a pixel to be marked as "stale". This means it will be skipped in future updates
 * until nearby pixels are swapped.
 * 3 seems sufficient
 */
const int STALE_TIME = 7;
/**
 * The width in pixels of the game world
 */
const int WORLD_WIDTH = 512;
/**
 * The height in pixels of the game world
 */
const int WORLD_HEIGHT = WORLD_WIDTH * 0.5625;
/**
 *  The maximum number of updates in a single tick
 */
const int MAX_UPDATES = 16;
/**
 *  Update loop will sleep after each update to ensure it doesn't exceed target FPS
 */
const int TARGET_FPS = 30;

/**
 *  Number of substeps in rigidbody update loop
 */
const int B2D_SUBSTEP_COUNT = 4;

/**
 *  Cap on how many rigidbody objects can exist
 */
const int MAX_OBJECTS = 512;
#endif
