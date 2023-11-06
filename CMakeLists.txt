cmake_minimum_required(VERSION 3.0.2)
project(test_dma)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_subdirectory(lib/example_driver)
add_subdirectory(lib/axidma)

include_directories(
  include
)

add_executable(${PROJECT_NAME} src/test_dma.cpp)
add_dependencies(${PROJECT_NAME} example_driver axidma)
target_link_libraries(${PROJECT_NAME} example_driver axidma)