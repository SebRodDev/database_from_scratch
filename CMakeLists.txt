cmake_minimum_required(VERSION 3.15)

# Set language to CXX (C++) since you are using C++ classes
project(database_from_scratch LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(.)
include_directories(src)

# Define your executable using your main source file, NOT just headers
add_executable(database_from_scratch
        main.cpp
        main.cpp
        # Add other .cpp files here as you create them (e.g., src/BPlusTree.cpp if you split it later)
)
add_executable(BPlusTree src/BPlusTree.h
        main.cpp
        main.cpp)