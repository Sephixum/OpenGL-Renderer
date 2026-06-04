cmake_minimum_required(VERSION 4.0)

project(
    glr
    VERSION "0.0.1"
    DESCRIPTION "OpenGL Renderer"
    LANGUAGES C CXX
)


set(CMAKE_SKIP_PACKAGE_ALL_DEPENDENCY ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)



add_subdirectory(third-patry)
add_subdirectory(src)

enable_testing()
include(CTest)
add_subdirectory(tests)
