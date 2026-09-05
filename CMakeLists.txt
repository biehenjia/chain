cmake_minimum_required(VERSION 3.20)
project(chain CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory(../link "${CMAKE_BINARY_DIR}/cr-core")

add_library(cr_algebra INTERFACE)
add_library(cr::algebra ALIAS cr_algebra)
target_include_directories(cr_algebra INTERFACE include)
target_link_libraries(cr_algebra INTERFACE cr_core)
