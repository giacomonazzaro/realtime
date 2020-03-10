#include <stdio.h>

#include <vector>
using namespace std;

#include "../yocto_modelio.h"
#include "../yocto_opengl.h"
#include "../yocto_window.h"
using namespace yocto;
using namespace opengl;

int main(int num_args, const char* args[]) {
  auto window = Window{};
  init_window(window, {400, 400}, "Window");

  auto mesh   = load_shape(args[1]);
  auto shader = create_program("shaders/mesh.vert", "shaders/mesh.frag");
  auto shape  = make_mesh(mesh.triangles, mesh.positions, mesh.normals);
  auto camera = make_lookat_camera({3, 3, 3}, {0, 0, 0});
  auto draw   = [&](Window& win) {
    clear_framebuffer({0, 0, 0, 1});
    update_camera(camera.frame, camera.focus, window);
    auto view       = make_view_matrix(camera);
    auto projection = make_projection_matrix(camera, window.size);
    // clang-format off
    draw_shape(shape, shader,
      Uniform("color", vec3f(1, 1, 1)),
      Uniform("frame", identity4x4f),
      Uniform("view", view),
      Uniform("projection", projection)
    );
    // clang-format on
  };

  run_draw_loop(window, draw);
}
