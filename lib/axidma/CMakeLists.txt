cmake_minimum_required(VERSION 3.0.2)
project(axidma)

# set_target_properties(dma_driver PROPERTIES LINKER_LANGUAGE CXX)

include_directories(
  include
)

add_library(axidma src/axidma.cpp )