#include <stdio.h>

#include <vector>
using namespace std;

#include "../yocto_modelio.h"
#include "../yocto_opengl.h"
#include "../yocto_window.h"
using namespace yocto;
using namespace opengl;

int main(int num_args, const char *args[]) {
  // Init window.
  auto win      = opengl_window();
  auto viewport = vec2i{300, 300};
  init_glwindow(win, viewport, __FILE__, nullptr);

  // Init shape.
  auto mesh  = load_shape("data/stanford-bunny.obj");
  auto shape = opengl_shape{};
  init_glshape(shape);
  add_vertex_attribute(shape, mesh.positions);
  add_vertex_attribute(shape, mesh.normals);
  init_elements(shape, mesh.triangles);

  // Init shader.
  auto shader = create_glprogram("shaders/mesh.vert", "shaders/mesh.frag");

  // Init camera.
  auto camera  = opengl_camera{};
  camera.frame = lookat_frame({10, 10, 10}, {0, 0, 0});

  // Draw.
  while (!draw_loop(win, true)) {
    clear_glframebuffer({0, 0, 1, 1});

    bind_glprogram(shader);
    set_gluniform(shader, "frame", identity4x4f);
    set_gluniform(shader, "view", make_view_matrix(camera));
    set_gluniform(
        shader, "projection", make_projection_matrix(camera, viewport));
    draw_glshape(shape);
  }

  delete_glwindow(win);
}
