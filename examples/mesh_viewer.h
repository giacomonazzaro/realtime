#include <stdio.h>

#include <vector>
using namespace std;

#include "../source/yocto_modelio.h"
#include "../source/yocto_opengl.h"
#include "../source/yocto_window.h"
using namespace yocto;
using namespace opengl;

vector<vec3f> compute_normals(
    const vector<vec3i>& triangles, const vector<vec3f>& positions) {
  auto normals = vector<vec3f>{positions.size()};
  for (auto& normal : normals) normal = zero3f;
  for (auto& t : triangles) {
    auto normal = cross(
        positions[t.y] - positions[t.x], positions[t.z] - positions[t.x]);
    normals[t.x] += normal;
    normals[t.y] += normal;
    normals[t.z] += normal;
  }
  for (auto& normal : normals) normal = normalize(normal);
  return normals;
}

inline Camera make_framing_camera(const vector<vec3f>& positions) {
  auto direction = vec3f{0, 1, 2};
  auto box       = bbox3f{};
  for (auto& p : positions) {
    expand(box, p);
  }
  auto box_center = center(box);
  auto box_size   = max(size(box));
  return make_lookat_camera(direction * box_size + box_center, box_center);
}

inline void run_viewer(const ioshape& mesh) {
  // Init window.
  auto win = Window();

  init_window(win, {300, 300}, "mesh viewer");

  // Init camera.
  auto camera = make_framing_camera(mesh.positions);

  // Init shape.
  auto normals = mesh.normals.empty()
                     ? compute_normals(mesh.triangles, mesh.positions)
                     : mesh.normals;
  auto shape = make_mesh(mesh.triangles, mesh.positions, normals);

  // Init shader.
  auto shader = create_shader("shaders/mesh.vert", "shaders/mesh.frag", true);

  auto draw = [&](Window& win) {
    update_camera(camera.frame, camera.focus, win);

    auto view       = make_view_matrix(camera);
    auto projection = make_projection_matrix(camera, win.size);

    // clang-format off
    clear_framebuffer(vec4f(0, 0, 0, 1));
    draw_shape(shape, shader,
      Uniform("color", vec3f(1, 1, 1)),
      Uniform("frame", identity4x4f),
      Uniform("view", view),
      Uniform("projection", projection)
    );
    // clang-format on
  };

  run_draw_loop(win, draw);

  delete_shape(shape);
  delete_shader(shader);
  delete_window(win);
}

inline void run_viewer(const string& filename) {
  run_viewer(load_shape(filename));
}