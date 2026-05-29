cmake_minimum_required(VERSION 3.16)

project(opengl_dodecahedron LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(OpenGL REQUIRED)
find_package(glfw3 CONFIG REQUIRED)

add_executable(opengl_dodecahedron
    opengl_dodecahedron.cpp
)

target_link_libraries(opengl_dodecahedron PRIVATE
    OpenGL::GL
    glfw
)