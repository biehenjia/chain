cmake_minimum_required(VERSION 3.20)
project(chain CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory(../link "${CMAKE_BINARY_DIR}/cr-core")

add_library(cr_algebra INTERFACE)
add_library(cr::algebra ALIAS cr_algebra)
target_include_directories(cr_algebra INTERFACE include)
target_link_libraries(cr_algebra INTERFACE cr_core)

include(CTest)
if(BUILD_TESTING)
    add_executable(cr_cleanup_test tests/cleanup.cpp)
    target_link_libraries(cr_cleanup_test PRIVATE cr::algebra)
    add_test(NAME cr_cleanup COMMAND cr_cleanup_test)
endif()

if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/tools/lsp.cpp")
    add_library(cr_lsp OBJECT EXCLUDE_FROM_ALL tools/lsp.cpp)
    target_link_libraries(cr_lsp PRIVATE cr::algebra)
endif()
