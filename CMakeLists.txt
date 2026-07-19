cmake_minimum_required(VERSION 3.20)
project(chains CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

include(CheckIPOSupported)
check_ipo_supported(RESULT ipo_supported)
if(ipo_supported)
    set(CMAKE_INTERPROCEDURAL_OPTIMIZATION_RELEASE ON)
endif()

add_compile_options($<$<CONFIG:Release>:-march=native>)

find_package(SymEngine CONFIG REQUIRED)

add_library(chains STATIC src/algebra_impl.cpp)
target_include_directories(chains PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_include_directories(chains PUBLIC ${SYMENGINE_INCLUDE_DIRS})
target_link_libraries(chains PUBLIC ${SYMENGINE_LIBRARIES})

add_executable(module_smoke .scratch/module_smoke.cpp)
target_link_libraries(module_smoke PRIVATE chains)

add_executable(materialize_smoke .scratch/materialize_smoke.cpp)
target_link_libraries(materialize_smoke PRIVATE chains)

add_executable(cmcr_numeric .scratch/cmcr_numeric.cpp)
target_link_libraries(cmcr_numeric PRIVATE chains)

add_executable(dump_tree .scratch/dump_tree.cpp)
target_link_libraries(dump_tree PRIVATE chains)

add_executable(sin_probe .scratch/sin_probe.cpp)
target_link_libraries(sin_probe PRIVATE chains)

add_executable(dump_tree_sym .scratch/dump_tree_sym.cpp)
target_link_libraries(dump_tree_sym PRIVATE chains)
