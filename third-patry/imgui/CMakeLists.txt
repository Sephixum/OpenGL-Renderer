cmake_minimum_required(VERSION 4.0)
project(imgui LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(imguilib STATIC)
target_sources(
    imguilib
    PRIVATE
        ${imgui_SOURCE_DIR}/imgui.cpp
        ${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp
        ${imgui_SOURCE_DIR}/backends/imgui_impl_glfw.cpp
        ${imgui_SOURCE_DIR}/imgui_demo.cpp
        ${imgui_SOURCE_DIR}/imgui_draw.cpp
        ${imgui_SOURCE_DIR}/imgui_tables.cpp
        ${imgui_SOURCE_DIR}/imgui_widgets.cpp
)
target_include_directories(
    imguilib
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/
)
# target_compile_definitions(
#     imguilib
#     PUBLIC
#         IMGUI_IMPL_OPENGL_LOADER_CUSTOM
# )
target_link_libraries(
    imguilib
    PUBLIC
        gladlib glfw
)
