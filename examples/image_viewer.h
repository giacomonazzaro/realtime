#include <stdio.h>

#include <vector>
using namespace std;

#include <graphics/image.h>
#include <realtime/gpu.h>
#include <realtime/window.h>
using namespace yocto;
using namespace window;
using namespace gpu;

inline void run_image_viewer(const image<vec4f>& image) {
  auto win = Window();

  // Init window, preserving image aspect ratio
  vec2i size = image.size();
  size.y *= 1000 / float(size.x);
  size.x = 1000;

  init_window(win, size, "image viewer");
  init_opengl();

  // Init gpu data.
  auto shape  = make_quad_shape();
  auto shader = make_shader_from_file(
      "shaders/quad.vert", "shaders/texture.frag");
  auto texture = Texture();
  init_texture(texture, image, true, false, true);

  auto draw = [&](Window& win) {
    clear_framebuffer({0, 0, 0, 1});
    bind_shader(shader);
    set_uniform_texture(shader, "image", texture, 0);
    draw_shape(shape);
  };

  run_draw_loop(win, draw);

  delete_shape(shape);
  delete_shader(shader);
  delete_window(win);
}

inline void run_image_viewer(const string& filename) {
  run_image_viewer(load_image(filename));
}
