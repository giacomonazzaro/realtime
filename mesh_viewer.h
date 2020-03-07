#include "yocto_modelio.h"
#include "yocto_opengl.h"
#include "yocto_window.h"
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

struct mesh_viewer {
  vec2i            viewport          = {500, 500};
  vec3f            background        = {0, 0, 0};
  string           vertex_filename   = "shaders/mesh.vert";
  string           fragment_filename = "shaders/mesh.frag";
  opengl_callbacks callbacks         = {};
};

inline void run(const mesh_viewer& viewer, const ioshape& mesh) {
  // Init window.
  auto win = opengl_window();
  init_glwindow(win, viewer.viewport, "mesh viewer");
  win.callbacks = viewer.callbacks;

  // Init shape.
  auto normals = mesh.normals.empty()
                     ? compute_normals(mesh.triangles, mesh.positions)
                     : mesh.normals;
  auto shape     = make_glmesh(mesh.triangles, mesh.positions, normals);
  auto direction = vec3f{0, 1, 2};
  auto box       = bbox3f{};
  for (auto& p : mesh.positions) {
    expand(box, p);
  }
  auto box_center = center(box);
  auto box_size   = max(size(box));

  // Init shader.
  auto shader = create_glprogram(
      viewer.vertex_filename, viewer.fragment_filename);

  // Init camera.
  auto camera = make_lookat_camera(
      direction * box_size + box_center, box_center);

  auto vector_field = make_glvector_field(normals, mesh.positions, 0.01);

  // Draw.
  while (!draw_loop(win)) {
    clear_glframebuffer(vec4f(viewer.background, 1));
    update_camera(camera.frame, camera.focus, win);
    auto view       = make_view_matrix(camera);
    auto projection = make_projection_matrix(camera, viewer.viewport);

    // clang-format off
    draw_glshape_cool(shape, shader,
      opengl_uniform{"color", vec3f{1, 0, 0}},
      opengl_uniform{"frame", identity4x4f},
      opengl_uniform{"view", view},
      opengl_uniform{"projection", projection}
    );
    // clang-format on
  }

  delete_glwindow(win);
}
