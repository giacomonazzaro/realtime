#include <stdio.h>

#include <vector>
using namespace std;

#include <graphics/image.h>
#include <realtime/gpu.h>
#include <realtime/window.h>
using namespace yocto;
using namespace window;
using namespace gpu;

inline void run_shadertoy(const string& filename) {
  // Init window.
  auto win = Window();

  init_window(win, {1000, 500}, "Shadertoy");
  init_opengl();

  auto quad   = make_quad_shape();
  auto shader = make_shader_from_file("shaders/quad.vert", filename);

  // Auto-recompile shader with window gains focus.
  win.callbacks.focus = [&shader](Window&, int focused) {
    if (focused) {
      printf("Reloading shader.\n");
      load_shader_code(shader);
      init_shader(shader);
    }
  };
  init_callbacks(win);

  auto draw = [&](Window& win) {
    clear_framebuffer({0, 0, 0, 1});

    // clang-format off
    draw_shape(quad, shader,
      Uniform("iTime", float(win.input.time_now)),
      Uniform("iFrame", win.input.frame),
      Uniform("iResolution", win.size),
      Uniform("iMouse", win.input.mouse_pos)
    );
    // clang-format on
  };

  run_draw_loop(win, draw, false);

  delete_shape(quad);
  delete_shader(shader);
  delete_window(win);
}
