# Set the minimum required version of CMake
cmake_minimum_required(VERSION 3.5)

# Project name
project(example)

# Set the output directory for libraries and binaries
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)

# Include directories
include_directories(${CMAKE_SOURCE_DIR}/include)

# Source files
file(GLOB SRC src/xexample_linux.c
src/xexample_sinit.c
src/xexample.c)
file(GLOB HEADERS src/xexample.h
src/xexample_hw.h)

# Create a library from source files
add_library(example_driver STATIC ${SRC})

# Specify the library output name and location
set_target_properties(example_driver PROPERTIES OUTPUT_NAME example_driver)
set_target_properties(example_driver PROPERTIES ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/lib)

# Copy headers to the include directory
foreach(HEADER ${HEADERS})
    get_filename_component(HEADER_NAME ${HEADER} NAME)
    configure_file(${HEADER} ${CMAKE_SOURCE_DIR}/include/${HEADER_NAME} COPYONLY)
endforeach()

# Print a message
message("Building ${PROJECT_NAME}")
