cmake_minimum_required(VERSION 3.16)
# Project name
project(FallingSandIter1 LANGUAGES CXX)

# Enabling this appends compile commands to compile_commands.json
set(CMAKE_EXPORT_COMPILE_COMMANDS NO)

# Set C++ standard - adjust as needed for your project
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find the OpenCL and graphics packages
find_package(OpenCL REQUIRED)
find_package(OpenGL REQUIRED)
find_package(GLEW REQUIRED)

find_package(glfw3 REQUIRED) 

# Find the sourcecode for the program
file(GLOB_RECURSE SOURCES "src/main/cpp/*.cpp")

add_executable(${PROJECT_NAME} ${SOURCES})

# Include directories, needs both headers and headers/include to ensure nested header files like rapidjson are reached by the compiler
target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src/main/headers
    ${CMAKE_CURRENT_SOURCE_DIR}/src/main/headers/include
)

# Linking all the libraries
target_link_libraries(${PROJECT_NAME} PRIVATE
    OpenCL::OpenCL
    GLEW::GLEW
    glfw
    OpenGL::GL
)
