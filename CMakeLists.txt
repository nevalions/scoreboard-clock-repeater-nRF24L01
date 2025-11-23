cmake_minimum_required(VERSION 3.16)

# Add extra component directory for radio-common
set(EXTRA_COMPONENT_DIRS ../radio-common)

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(repeater)