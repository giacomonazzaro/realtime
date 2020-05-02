cmake_minimum_required (VERSION 3.12)
project (realtime)

option(REALTIME_EXAMPLES "Build examples" ON)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  set(CMAKE_BUILD_TYPE “Release”)
endif()
set(CMAKE_BINARY_DIR ${CMAKE_SOURCE_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

# set_target_properties(realtime PROPERTIES CXX_STANDARD 17 CXX_STANDARD_REQUIRED YES)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory(realtime/ext/graphics)

add_library(realtime
    realtime/gpu.h realtime/gpu.cpp realtime/window.h realtime/window.cpp
    # font.h font.cpp
    realtime/ext/imgui/imgui.cpp realtime/ext/imgui/imgui_draw.cpp realtime/ext/imgui/imgui_widgets.cpp
    realtime/ext/imgui/imgui_impl_glfw.cpp realtime/ext/imgui/imgui_impl_opengl3.cpp realtime/ext/glad/glad.c)


if(APPLE)
    target_compile_options(realtime PRIVATE -Wall)
    include_directories(/usr/local/include /usr/local/include/freetype2)
    link_directories(/usr/local/lib)
    find_library(GLFW_LIBRARY NAMES glfw3 glfw PATHS /usr/local/lib)
    set(GL_EXTRA_LIBRARIES)
endif(APPLE)

if(WIN32)
    target_compile_options(yocto_gui PRIVATE /D_CRT_SECURE_NO_WARNINGS /wd4018 /wd4244 /wd4305 /wd4800 /wd4267)
    target_compile_options(yocto_gui PUBLIC /EHsc)
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


target_link_libraries(realtime graphics)
target_include_directories(realtime PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")


if(REALTIME_EXAMPLES)
    add_executable(mesh_viewer ./examples/mesh_viewer.cpp ./examples/mesh_viewer.h)
    target_link_libraries(mesh_viewer realtime ${OPENGL_gl_LIBRARY} ${GLFW_LIBRARY} ${GL_EXTRA_LIBRARIES})

    add_executable(image_viewer ./examples/image_viewer.cpp ./examples/image_viewer.h)
    target_link_libraries(image_viewer realtime ${OPENGL_gl_LIBRARY} ${GLFW_LIBRARY} ${GL_EXTRA_LIBRARIES})

    add_executable(shadertoy ./examples/shadertoy.cpp ./examples/shadertoy.h)
    target_link_libraries(shadertoy realtime ${OPENGL_gl_LIBRARY} ${GLFW_LIBRARY} ${GL_EXTRA_LIBRARIES})

    add_executable(deferred ./examples/deferred_shading.cpp)
    target_link_libraries(deferred realtime ${OPENGL_gl_LIBRARY} ${GLFW_LIBRARY} ${GL_EXTRA_LIBRARIES})
endif(REALTIME_EXAMPLES)
