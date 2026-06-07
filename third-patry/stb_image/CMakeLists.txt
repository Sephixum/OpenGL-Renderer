cmake_minimum_required(VERSION 4.0)

project(
    glad
    DESCRIPTION "stb image loader"
)

add_library(stbimagelib STATIC)
target_sources(stbimagelib PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/stb_image.c)
target_include_directories(stbimagelib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
