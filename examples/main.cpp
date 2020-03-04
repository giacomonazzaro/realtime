#include <stdio.h>

#include <vector>
using namespace std;

#include "../yocto_modelio.h"
#include "../yocto_opengl.h"
#include "../yocto_window.h"
using namespace yocto;

int main(int num_args, const char *args[]) {
  auto win = opengl_window();
  init_glwindow(win, {300, 300}, __FILE__, nullptr);

  auto quad = opengl_shape{};
  init_glquad(quad);

  auto shader = create_glprogram("shaders/quad.vert", "shaders/quad.frag");

  auto shape = load_shape("data/stanford-bunny.obj");

  while (!draw_loop(win, true)) {
    clear_glframebuffer({0, 0, 1, 1});

    bind_glprogram(shader);
    draw_glshape(quad);
  }

  delete_glwindow(win);
}
