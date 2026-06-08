add_library(glrlib STATIC)

add_subdirectory(Utils)
add_subdirectory(Graphics)
add_subdirectory(Services)
add_subdirectory(Systems)
add_subdirectory(Core)

add_executable(app)
target_sources(app PRIVATE Main.cpp)
target_compile_options(
     app
     PRIVATE
        -Wall
        -Wextra
        -pedantic
        -Werror
        -Wconversion-null
        -Wmissing-declarations
        -Woverlength-strings
        -Wpointer-arith
        -Wunused-local-typedefs
        -Wunused-result
        -Wvarargs
        -Wvla
        -Wwrite-strings
        -Wno-missing-declarations
        -O0
        -ggdb
)
target_compile_options(
    glrlib
    PRIVATE
        -Wall
        -Wextra
        -pedantic
        -Werror
        -Wconversion-null
        -Wmissing-declarations
        -Woverlength-strings
        -Wpointer-arith
        -Wunused-local-typedefs
        -Wunused-result
        -Wvarargs
        -Wvla
        -Wwrite-strings
        -O0
        -ggdb
)

target_include_directories(glrlib PUBLIC ${PROJECT_SOURCE_DIR}/ ${PROJECT_SOURCE_DIR}/src)
target_compile_features(glrlib PUBLIC cxx_std_23)
target_compile_definitions(glrlib PUBLIC NOMINMAX)
target_link_libraries(glrlib PUBLIC stdc++exp gladlib imguilib imguizmo assimp ImGuiFileDialog stbimagelib)
target_link_libraries(app PUBLIC glrlib)
