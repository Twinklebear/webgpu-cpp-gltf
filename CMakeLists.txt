cmake_minimum_required(VERSION 3.27)
project(wgpu-cpp-wasm)
# Library version for our npm package
set(LIBRARY_VERSION "0.2.3")

if(NOT WIN32)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -pedantic")
endif()

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_LIST_DIR}/cmake")
include(ExternalProject)
include(${CMAKE_CURRENT_LIST_DIR}/cmake/EmbedFile.cmake)

include(cmake/glm.cmake)

add_definitions(-DGLM_ENABLE_EXPERIMENTAL)

add_subdirectory(src)
