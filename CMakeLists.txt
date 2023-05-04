cmake_minimum_required(VERSION 3.13)  # CMake version check
project(test)               # Create project "simple_example"
set(CMAKE_CXX_STANDARD 17)            # Enable c++14 standard

# Add main.cpp file of project root directory as source file
# set(SOURCE_FILES main.cpp FormulaParser.cpp)

# Add executable target with source files listed in SOURCE_FILES variable
add_executable(test main.cpp FormulaParser.cpp)

target_link_libraries(test /usr/local/lib/libboost_filesystem.a)