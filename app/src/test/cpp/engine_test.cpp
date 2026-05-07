// app/src/test/cpp/simulation_test.cpp
#include <format>
#include <iostream>
#include <cassert>
#include <cstring>
#include <string>
#include "CL/opencl.hpp"

// Include your application headers
#include "engine.h"
#include "buffer.h"
#include "cl_setup.h"
#include "graphics.h"
int run = 1;
// Simple test framework (you can replace with Google Test later)
#define TEST_ASSERT(condition, message) \
    if (!(condition)) { \
        std::cerr << std::endl << "FAILED: " << message << " (line " << __LINE__ << ")" << std::endl; \
        return false; \
    }

#define TEST_RUN(name) \
    std::cout << std::endl << "Running test: " << run << "... "; \
    run++; \
    if (!name()) { \
        std::cout << std::endl << "FAILED" << std::endl; \
        return 1; \
    } \
    std::cout << std::endl << "PASSED" << std::endl;

// Test 1: Buffer initialization
bool test_buffer_initialization() {
    int width = 10;
    int height = 10;
    
    char* buf = init_buf(width, height);
    
    // Check all cells are spaces
    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            TEST_ASSERT(buf[i * width + j] == ' ', 
                       "Buffer cell should be space");
        }
    }
    
    delete[] buf;
    return true;
}

// Test 2: Output buffer format
bool test_output_buffer_format() {
    int width = 5;
    int height = 3;
    
    char* output_buf = init_output_buf(width, height);
    int output_width = width + 3;
    
    // Check top border
    TEST_ASSERT(output_buf[0] == '-', "Top border should be '-'");
    TEST_ASSERT(output_buf[output_width - 1] == '\n', "Should end with newline");
    
    std::string text = "Left border should be '|', found\"\"";
    // Check left border
    std::cout << output_buf[9] << std::endl;
    TEST_ASSERT(output_buf[output_width] == '|', 
            text.insert(text.size()-1,{output_buf[output_width + 1]}));
    
    
    delete[] output_buf;
    return true;
}

// Test3: Single particle falling. Makes sure it moves 1 cell down
bool test_single_particle_falling() {
    const int width = 10;
    const int height = 10;
    const int n_width = 2;
    const int n_height = 2;
    
    char* buf = init_buf(width, height);
    buf[0] = 'S';
    
    char* output_buf = init_output_buf(width, height);
    
    CL cl(buf, width, height);
    
    cl.setBufferArg(0);
    cl.setArg(2, width);
    cl.setArg(3, height);
    cl.setArg(4, n_width);
    cl.setArg(5, n_height);
    
    int iteration = 0;

    
    update(&cl, &iteration, 1, width, height, n_width, n_height, buf, output_buf);
    

    bool particle_moved = (buf[0] != 'S');
    bool particle_in_correct_location = (buf[width] == 'S');
    TEST_ASSERT(iteration == 1, "Iteration should update");
    TEST_ASSERT(particle_moved, "Particle should have moved after 1 iteration");
    TEST_ASSERT(particle_in_correct_location, "Particle should have moved down 1 after 1 iteration");
    
    delete[] buf;
    delete[] output_buf;
    return true;
}

// Test4: Single particle falling
bool test_single_particle_falling_multiple() {
    const int width = 10;
    const int height = 10;
    const int n_width = 2;
    const int n_height = 2;
    
    char* buf = init_buf(width, height);
    buf[0] = 'S';
    
    char* output_buf = init_output_buf(width, height);
    
    CL cl(buf, width, height);
    
    cl.setBufferArg(0);
    cl.setArg(2, width);
    cl.setArg(3, height);
    cl.setArg(4, n_width);
    cl.setArg(5, n_height);
    
    int iteration = 0;

    
    update(&cl, &iteration, 4, width, height, n_width, n_height, buf, output_buf);
    

    bool particle_moved = (buf[0] != 'S');
    bool particle_in_correct_location = (buf[width * 4] == 'S');
    TEST_ASSERT(iteration == 4, "Iteration should update");
    TEST_ASSERT(particle_moved, "Particle should have moved after 4 iterations");
    TEST_ASSERT(particle_in_correct_location, "Particle should have moved down 4 after 4 iteration");
    
    delete[] buf;
    delete[] output_buf;
    return true;
}

// Test5: Two particles falling
bool test_two_particles_falling() {
    const int width = 10;
    const int height = 10;
    const int n_width = 2;
    const int n_height = 2;
    
    char* buf = init_buf(width, height);
    buf[0] = 'S';
    buf[1] = 'S';
    
    char* output_buf = init_output_buf(width, height);
    
    CL cl(buf, width, height);
    
    cl.setBufferArg(0);
    cl.setArg(2, width);
    cl.setArg(3, height);
    cl.setArg(4, n_width);
    cl.setArg(5, n_height);
    
    int iteration = 0;

    
    update(&cl, &iteration, 1, width, height, n_width, n_height, buf, output_buf);
    

    bool particles_moved = (buf[0] != 'S' && buf[1] != 'S');
    bool particles_in_correct_location = (buf[width] == 'S' && buf[width+1] == 'S');
    TEST_ASSERT(iteration == 1, "Iteration should update");
    TEST_ASSERT(particles_moved, "Particles should have moved after 1 iteration");
    TEST_ASSERT(particles_in_correct_location, "Particles should have moved down 1 after 1 iteration");
    
    delete[] buf;
    delete[] output_buf;
    return true;
}

// Test6: Two particles falling multiple steps
bool test_two_particles_falling_multiple() {
    const int width = 10;
    const int height = 10;
    const int n_width = 2;
    const int n_height = 2;
    
    char* buf = init_buf(width, height);
    buf[0] = 'S';
    buf[1] = 'S';
    
    char* output_buf = init_output_buf(width, height);
    
    CL cl(buf, width, height);
    
    cl.setBufferArg(0);
    cl.setArg(2, width);
    cl.setArg(3, height);
    cl.setArg(4, n_width);
    cl.setArg(5, n_height);
    
    int iteration = 0;

    
    update(&cl, &iteration, 4, width, height, n_width, n_height, buf, output_buf);
    

    bool particles_moved = (buf[0] != 'S' && buf[1] != 'S');
    bool particles_in_correct_location = (buf[width*4] == 'S' && buf[(width*4)+1] == 'S');
    TEST_ASSERT(iteration == 4, "Iteration should update");
    TEST_ASSERT(particles_moved, "Particles should have moved after 4 iterations");
    TEST_ASSERT(particles_in_correct_location, "Particles should have moved down 4 after 4 iteration");
    
    delete[] buf;
    delete[] output_buf;
    return true;
}

bool is_expected(int width, int height, char* initial_state, char* expected_state, int number_of_steps) {

    const int n_width = 2;
    const int n_height = 2;
    
    char* buf = initial_state;
    
    char* output_buf = init_output_buf(width, height);
    
    CL cl(buf, width, height);
    
    cl.setBufferArg(0);
    cl.setArg(2, width);
    cl.setArg(3, height);
    cl.setArg(4, n_width);
    cl.setArg(5, n_height);
    
    int iteration = 0;

    
    update(&cl, &iteration, number_of_steps, width, height, n_width, n_height, buf, output_buf);
    
    std::cout << std::endl;     
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            std::cout << x << "," << y << ": got [" << buf[y*width + x] << "] , expected [" << expected_state[y * width + x] << "]" << std::endl;
            TEST_ASSERT(buf[y * width + x] == expected_state[y * width + x], "Difference between calculated and expected states");
        }
    }
    
    delete[] buf;
    delete[] output_buf;
    return true;
}

bool is_expected_iterative(int width, int height, char* initial_state, char* expected_state, int number_of_steps) {

    const int n_width = 2;
    const int n_height = 2;
    
    char* buf = initial_state;
    
    char* output_buf = init_output_buf(width, height);
    
    CL cl(buf, width, height);
    
    cl.setBufferArg(0);
    cl.setArg(2, width);
    cl.setArg(3, height);
    cl.setArg(4, n_width);
    cl.setArg(5, n_height);
    
    int iteration = 0;

    
    for (int i = 0; i < number_of_steps; i++) {
        update(&cl, &iteration, number_of_steps, width, height, n_width, n_height, buf, output_buf);
    }
    
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            std::cout << x << "," << y << buf[y*width + x] << " : " << expected_state[y * width + x] << std::endl;
            TEST_ASSERT(buf[y * width + x] == expected_state[y * width + x], "Difference between calculated and expected states");
        }
    }
    
    delete[] buf;
    delete[] output_buf;

    return true;
}

// Test 7: testing boundary case
bool test_bottom_boundary_one_sand() {
    char* initial = new char[]{
        ' ',' ',
        'S',' '
    };
    char* expected = new char[]{
        ' ',' ',
        'S',' '
    };
    return is_expected(2,2,initial,expected,1);
}

// Tests if sand will fall to right
bool test_bottom_boundary_two_sand() {
    
    char* initial = new char[]{
        'S',' ',
        'S',' '
    };
    char* expected = new char[]{
        ' ',' ',
        'S','S'
    };
    return is_expected(2,2,initial,expected,5);
}

// Main test runner
int main() {
    std::cout << "========================================" << std::endl;
    std::cout << "Running Simulation Tests" << std::endl;
    std::cout << "========================================" << std::endl;
    //
    TEST_RUN((void*)test_buffer_initialization);
    TEST_RUN((void*)test_output_buffer_format);
    TEST_RUN((void*)test_single_particle_falling);
    TEST_RUN((void*)test_single_particle_falling_multiple);
    TEST_RUN((void*)test_two_particles_falling);
    TEST_RUN((void*)test_two_particles_falling_multiple);
    TEST_RUN((void*)test_bottom_boundary_one_sand);
    TEST_RUN((void*)test_bottom_boundary_two_sand);
    // TEST_RUN(test_multiple_iterations);
    // TEST_RUN(test_boundary_conditions);
    // TEST_RUN(test_kernel_argument_consistency);
    //
    // std::cout << "========================================" << std::endl;
    // std::cout << "All tests passed!" << std::endl;
    // std::cout << "========================================" << std::endl;
    
    return 0;
}
