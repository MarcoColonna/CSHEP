cmake_minimum_required(VERSION 3.16)
project(mandelbrot VERSION 0.1.0)

# Find SFML and TBB
find_package(SFML 2.5 COMPONENTS graphics REQUIRED)
find_package(TBB REQUIRED)

# Require C++17, without non-standard compiler extensions
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS ON)

# Always compile with warnings enabled
string(APPEND CMAKE_CXX_FLAGS " -Wall -Wextra")

# Enable the address sanitizer in debug mode
string(APPEND CMAKE_CXX_FLAGS_DEBUG " -fsanitize=address -fno-omit-frame-pointer")
string(APPEND CMAKE_EXE_LINKER_FLAGS_DEBUG " -fsanitize=address -fno-omit-frame-pointer")

# Add the source file and build the executable
add_executable(mandelbrot main.cpp)

# Link the required libraries
target_link_libraries(mandelbrot PRIVATE sfml-graphics TBB::tbb)

