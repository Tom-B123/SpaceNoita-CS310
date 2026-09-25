cmake_minimum_required(VERSION 3.16)
project(FallingSandIter1 LANGUAGES CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS OFF)

# Set C++ standard - adjust as needed for your project
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# --- Find Required Libraries ---
# This is where CMake shines compared to your Gradle setup
find_package(OpenCL REQUIRED)
find_package(OpenGL REQUIRED)
find_package(GLEW REQUIRED)

# For GLFW, the Find Module varies by system. 
# On Arch, it's often best to use pkg-config or the GLFW3 find module.
find_package(glfw3 REQUIRED) 
# If the above fails, you might need: find_package(PkgConfig REQUIRED) && pkg_check_modules(GLFW REQUIRED glfw3)

# --- Define Your Executable ---
# This recursively finds all .cpp files in your src directory.
# You can also list files explicitly if you prefer.
file(GLOB_RECURSE SOURCES "src/main/cpp/*.cpp")

add_executable(${PROJECT_NAME} ${SOURCES})

# --- Specify Include Directories ---
# For system libraries, find_package usually handles this automatically.
# You only need this if you have your own private headers.
target_include_directories(${PROJECT_NAME} PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}/src/main/headers
)

# --- Link Libraries ---
# The imported targets (OpenCL::OpenCL, GLEW::GLEW, etc.) automatically 
# handle include paths and linker flags for you.
target_link_libraries(${PROJECT_NAME} PRIVATE
    OpenCL::OpenCL
    GLEW::GLEW
    glfw
    OpenGL::GL
)
