cmake_minimum_required(VERSION 3.20)
project(chains CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(SymEngine CONFIG REQUIRED)

add_library(chains STATIC src/algebra.cpp)
target_include_directories(chains
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${SYMENGINE_INCLUDE_DIRS}
)
target_link_libraries(chains PUBLIC ${SYMENGINE_LIBRARIES})

enable_testing()
add_executable(test_arena tests/test_arena.cpp)
target_link_libraries(test_arena PRIVATE chains)
add_test(NAME test_arena COMMAND test_arena)
