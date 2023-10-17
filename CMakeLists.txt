cmake_minimum_required(VERSION 3.20.0)
project(
    CAPS
    VERSION 01.07
    DESCRIPTION "CAPS CMake built with VSCode"
)
set(CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 20)

add_subdirectory(src)

include(CTest)
enable_testing()
