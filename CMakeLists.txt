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
    
    # add_library(box2d
    #     source/ext/box2d-lite/src/Arbiter.cpp source/ext/box2d-lite/src/Body.cpp source/ext/box2d-lite/src/Collide.cpp source/ext/box2d-lite/src/Joint.cpp source/ext/box2d-lite/src/World.cpp
    # )


    add_library(yocto_opengl
        source/yocto_opengl.h source/yocto_opengl.cpp source/yocto_image.cpp source/yocto_modelio.cpp source/yocto_window.cpp
        # yocto_font.h yocto_font.cpp
        source/ext/imgui/imgui.cpp source/ext/imgui/imgui_draw.cpp source/ext/imgui/imgui_widgets.cpp
        source/ext/imgui/imgui_impl_glfw.cpp source/ext/imgui/imgui_impl_opengl3.cpp source/ext/glad/glad.c)


# include_directories(“${PROJECT_SOURCE_DIR}/../yocto-gl”)
# add_subdirectory (source/ext/yocto-gl)
# add_subdirectory (source/ext/box2d-lite)
# include_directories (source/ext)
add_executable(test examples/main.cpp)
target_link_libraries(test yocto_opengl ${OPENGL_gl_LIBRARY} ${GLFW_LIBRARY} ${GL_EXTRA_LIBRARIES})

add_executable(symchem examples/symchem.cpp)
target_link_libraries(symchem yocto_opengl ${OPENGL_gl_LIBRARY} ${GLFW_LIBRARY} ${GL_EXTRA_LIBRARIES})

add_executable(csg examples/csg.cpp)
target_link_libraries(csg yocto_opengl ${OPENGL_gl_LIBRARY} ${GLFW_LIBRARY} ${GL_EXTRA_LIBRARIES})