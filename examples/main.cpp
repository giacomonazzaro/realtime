#include <stdio.h>
#include <vector>
using namespace std;

#include "../yocto_opengl.h"
#include "../yocto_modelio.h"
using namespace yocto;

int main(int num_args, const char *args[]) {
    auto win = opengl_window();
    init_glwindow(win, {300, 300}, __FILE__, nullptr);

    auto quad = opengl_shape{};
    init_glquad(quad);

    string vertex = R"(
        #version 330
        layout(location = 0) in vec2 pos;
        void main() { gl_Position = vec4(pos.xy, 0, 1); }
        )";

    string fragment = R"(
        #version 330
        out vec4 color;
        void main() { color = vec4(1, 1, 0, 1); }
        )";
    
    auto shader = opengl_program{};
    init_glprogram(shader, vertex.c_str(), fragment.c_str(), false);

    auto shape = load_shape("data/stanford-bunny.obj");

    while(!should_glwindow_close(win)) {
        clear_glframebuffer({0, 0, 1, 1});
        
        bind_glprogram(shader);
        draw_glshape(quad);

        swap_glbuffers(win);
        process_glevents(win, true /* wait */);
    }

    delete_glwindow(win);
}