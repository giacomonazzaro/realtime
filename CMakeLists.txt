cmake_minimum_required (VERSION 3.9)
project (my-opengl)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  set(CMAKE_BUILD_TYPE “Release”)
endif()
set(CMAKE_BINARY_DIR ${CMAKE_SOURCE_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

if(APPLE)
    include_directories(/usr/local/include /usr/local/include/freetype2)
    link_directories(/usr/local/lib)
    find_library(GLFW_LIBRARY NAMES glfw3 glfw PATHS /usr/local/lib)
    set(GL_EXTRA_LIBRARIES)
endif(APPLE)
if(WIN32)
    include_directories(${CMAKE_SOURCE_DIR}/source/ext/glfw_w32/include)
    link_directories(${CMAKE_SOURCE_DIR}/source/ext/glfw_w32/lib-vc2015)
    find_library(GLFW_LIBRARY NAMES glfw3dll PATHS ${CMAKE_SOURCE_DIR}/source/ext/glfw_w32/lib-vc2015)
    set(GL_EXTRA_LIBRARIES)
endif(WIN32)
if(UNIX AND NOT APPLE)
    include_directories(/usr/include /usr/local/include)
    find_library(GLFW_LIBRARY NAMES glfw3 glfw PATHS /usr/lib /usr/local/lib64 /usr/lib64 /usr/local/lib /usr/lib/x86_64-linux-gnu)
    set(GL_EXTRA_LIBRARIES dl)
endif(UNIX AND NOT APPLE)
add_definitions(-DIMGUI_IMPL_OPENGL_LOADER_GLAD)

add_subdirectory(opengl/ext/graphics)

add_library(opengl
    opengl/yocto_opengl.h opengl/yocto_opengl.cpp opengl/yocto_window.h opengl/yocto_window.cpp
    # yocto_font.h yocto_font.cpp
    opengl/ext/imgui/imgui.cpp opengl/ext/imgui/imgui_draw.cpp opengl/ext/imgui/imgui_widgets.cpp
    opengl/ext/imgui/imgui_impl_glfw.cpp opengl/ext/imgui/imgui_impl_opengl3.cpp opengl/ext/glad/glad.c)

target_link_libraries(opengl graphics)


target_include_directories(opengl PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")

target_compile_definitions(opengl PUBLIC OPENGL_VERSION=1)

# 
#       How to link this library!!!
# 
# target_link_libraries(main opengl ${OPENGL_gl_LIBRARY} ${GLFW_LIBRARY} ${GL_EXTRA_LIBRARIES})
