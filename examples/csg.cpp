#include <stdio.h>

#include <vector>
using namespace std;

#include "../parser.h"
#include "../yocto_modelio.h"
#include "../yocto_opengl.h"
#include "../yocto_window.h"
using namespace yocto;

int main(int num_args, const char *args[]) {
  // Init window.
  auto win      = opengl_window();
  auto viewport = vec2i{300, 300};

  init_glwindow(win, viewport, __FILE__, nullptr);
  set_glblending(true);

  // Init shape.
  auto mesh  = load_shape("data/cube.obj");
  auto shape = opengl_shape{};
  init_glshape(shape);
  add_vertex_attribute(shape, mesh.positions);
  if (mesh.normals.empty()) mesh.normals = mesh.positions;
  add_vertex_attribute(shape, mesh.normals);
  init_elements(shape, mesh.triangles);

  // Init shader.
  auto shader = create_glprogram("shaders/mesh.vert", "shaders/csg.frag");

  // Init camera.
  auto camera = opengl_camera{};

  // Init csg.
  auto from          = vec3f(2, 2, 2);
  auto to            = vec3f(0.5, 0.5, 0.5);
  camera.frame       = lookat_frame(from, to);
  float camera_focus = length(from - to);
  auto  csg          = load_csg("data/test.csg");

  bind_glprogram(shader);
  for (int i = 0; i < csg.nodes.size(); i++) {
    string pre = "nodes[" + std::to_string(i) + "].";
    set_gluniform(shader, (pre + "children").c_str(), csg.nodes[i].children);
    for (int k = 0; k < CSG_NUM_PARAMS; k++) {
      auto param = pre + "params[" + std::to_string(k) + "]";
      set_gluniform(shader, param.c_str(), csg.nodes[i].primitive.params[k]);
    }
    set_gluniform(
        shader, (pre + "type").c_str(), (int)csg.nodes[i].primitive.type);
  }

  vec2f mouse_pos, last_pos;
  // Draw.
  while (!draw_loop(win, true)) {
    clear_glframebuffer({0, 0, 1, 1});

    last_pos         = mouse_pos;
    mouse_pos        = get_glmouse_pos(win);
    auto mouse_left  = get_glmouse_left(win);
    auto mouse_right = get_glmouse_right(win);
    auto alt_down    = get_glalt_key(win);
    auto shift_down  = get_glshift_key(win);

    // handle mouse and keyboard for navigation
    if ((mouse_left || mouse_right) && !alt_down) {
      auto dolly  = 0.0f;
      auto pan    = zero2f;
      auto rotate = zero2f;
      if (mouse_left && !shift_down) rotate = (mouse_pos - last_pos) / 100.0f;
      if (mouse_right) dolly = (mouse_pos.x - last_pos.x) / 100.0f;
      if (mouse_left && shift_down)
        pan = (mouse_pos - last_pos) * camera_focus / 200.0f;
      pan.x = -pan.x;
      update_turntable(camera.frame, camera_focus, rotate, dolly, pan);
    }

    set_gluniform(shader, "num_nodes", (int)csg.nodes.size());
    set_gluniform(shader, "eye", camera.frame.o);
    set_gluniform(shader, "frame", identity4x4f);
    set_gluniform(shader, "view", make_view_matrix(camera));
    set_gluniform(
        shader, "projection", make_projection_matrix(camera, viewport));
    draw_glshape(shape);
  }

  delete_glwindow(win);
}
