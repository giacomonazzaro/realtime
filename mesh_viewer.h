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

struct mesh_viewer_options {
  vec2i            viewport          = {500, 500};
  vec4f            background        = {0, 0, 0, 1};
  string           vertex_filename   = "shaders/mesh.vert";
  string           fragment_filename = "shaders/mesh.frag";
  opengl_callbacks callbacks         = {};
};

inline void mesh_viewer(
    const ioshape& mesh, const mesh_viewer_options& options = {}) {
  // Init window.
  auto win = opengl_window();
  init_glwindow(win, options.viewport, "mesh viewer");
  win.callbacks = options.callbacks;

  // Init shape.
  auto shape = opengl_shape{};
  init_glshape(shape);
  auto normals = mesh.normals.empty()
                     ? compute_normals(mesh.triangles, mesh.positions)
                     : mesh.normals;
  add_vertex_attribute(shape, mesh.positions);
  add_vertex_attribute(shape, normals);
  init_elements(shape, mesh.triangles);
  auto direction = vec3f{0, 1, 2};
  auto box       = bbox3f{};
  for (auto& p : mesh.positions) {
    expand(box, p);
  }
  auto box_center = center(box);
  auto box_size   = max(size(box));

  // Init shader.
  auto shader = create_glprogram(
      options.vertex_filename, options.fragment_filename);

  // Init camera.
  auto camera = make_lookat_camera(
      direction * box_size + box_center, box_center);

  normals.resize(mesh.triangles.size());
  auto vector_field = make_glvector_field(
      normals, mesh.triangles, mesh.positions, 0.01);

  // Draw.
  while (!should_glwindow_close(win)) {
    clear_glframebuffer(options.background);

    bind_glprogram(shader);
    set_gluniform(shader, "frame", identity4x4f);
    set_gluniform(shader, "view", make_view_matrix(camera));
    set_gluniform(
        shader, "projection", make_projection_matrix(camera, options.viewport));
    set_gluniform(shader, "color", vec3f{1, 0, 0});
    draw_glshape(shape);
    set_gluniform(shader, "color", vec3f{1, 1, 1});
    draw_glshape(vector_field);

    swap_glbuffers(win);
    update_camera(camera.frame, camera.focus, win);
    process_glevents(win, true);
  }

  delete_glwindow(win);
}
