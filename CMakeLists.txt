cmake_minimum_required(VERSION 3.16)

# Add radio-common submodule
add_subdirectory(radio-common)

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(repeater)