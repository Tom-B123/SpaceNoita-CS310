cmake_minimum_required(VERSION 3.16)
# Project name
project(FallingSandIter1 LANGUAGES CXX)

# Enabling this appends compile commands to compile_commands.json
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find the OpenCL and graphics packages
find_package(OpenGL REQUIRED)

# Find the sourcecode for the program
file(GLOB_RECURSE SOURCES "src/main/cpp/*.cpp")

include(FetchContent)
FetchContent_Declare(
    box2d
    GIT_REPOSITORY https://github.com/erincatto/box2d.git
    GIT_TAG v3.2.0  # Or another v3.x version
    GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(box2d)

add_executable(${PROJECT_NAME} ${SOURCES})
target_link_libraries(FallingSandIter1 PRIVATE box2d::box2d)

# Tell the compiler that we are using static GLEW
if(WIN32)
    target_compile_definitions(${PROJECT_NAME} PRIVATE GLEW_STATIC)
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

    add_library(glfw UNKNOWN IMPORTED)
    set_target_properties(glfw PROPERTIES
        IMPORTED_LOCATION           "${CMAKE_CURRENT_SOURCE_DIR}/src/main/headers/lib/glfw3.lib"
        INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_CURRENT_SOURCE_DIR}/src/main/headers/include"
    )

else()
    find_package(OpenCL REQUIRED)
    find_package(GLEW REQUIRED)
    find_package(glfw3 REQUIRED) 
endif()

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
