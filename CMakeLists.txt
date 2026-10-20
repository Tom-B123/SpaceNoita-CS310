cmake_minimum_required(VERSION 3.16)
# Project name
project(FallingSandIter1 LANGUAGES CXX)

# Enabling this appends compile commands to compile_commands.json
set(CMAKE_EXPORT_COMPILE_COMMANDS OFF)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find the OpenCL and graphics packages
# find_package(OpenCL REQUIRED)
find_package(OpenGL REQUIRED)
# find_package(GLEW REQUIRED)
# find_package(glfw3 REQUIRED) 

# Find the sourcecode for the program
file(GLOB_RECURSE SOURCES "src/main/cpp/*.cpp")

add_executable(${PROJECT_NAME} ${SOURCES})

# Tell the compiler that we are using static GLEW
target_compile_definitions(${PROJECT_NAME} PRIVATE GLEW_STATIC)

add_library(OpenCL::OpenCL UNKNOWN IMPORTED)
set_target_properties(OpenCL::OpenCL PROPERTIES
    IMPORTED_LOCATION           "${CMAKE_CURRENT_SOURCE_DIR}/src/main/headers/lib/OpenCL.lib"
    INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_CURRENT_SOURCE_DIR}/src/main/headers/include"
)
add_library(GLEW::GLEW UNKNOWN IMPORTED)
set_target_properties(GLEW::GLEW PROPERTIES
    # Note: this is glew32s, the static (non dll) library
    IMPORTED_LOCATION           "${CMAKE_CURRENT_SOURCE_DIR}/src/main/headers/lib/glew32s.lib"
    INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_CURRENT_SOURCE_DIR}/src/main/headers/include"
)

add_library(GLFW::GLFW UNKNOWN IMPORTED)
set_target_properties(GLFW::GLFW PROPERTIES
    IMPORTED_LOCATION           "${CMAKE_CURRENT_SOURCE_DIR}/src/main/headers/lib/glfw3.lib"
    INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_CURRENT_SOURCE_DIR}/src/main/headers/include"
)
# Include directories, needs both headers and headers/include to ensure nested header files like rapidjson are reached by the compiler
target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src/main/headers
    ${CMAKE_CURRENT_SOURCE_DIR}/src/main/headers/include
)

# Linking all the libraries
target_link_libraries(${PROJECT_NAME} PRIVATE
    OpenCL::OpenCL
    GLEW::GLEW
    GLFW::GLFW
    OpenGL::GL
)
