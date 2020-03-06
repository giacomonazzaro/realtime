#include "yocto_modelio.h"
#include "yocto_opengl.h"
#include "yocto_window.h"
using namespace yocto;
using namespace opengl;

const char* vert = R"(
#version 330
layout(location = 0) in vec3 vposition;
layout(location = 1) in vec3 vnormal;
out vec3 position;
out vec3 normal;

uniform mat4 frame;
uniform mat4 view;
uniform mat4 projection;

void main() {
  position    = (frame * vec4(vposition, 1)).xyz;
  normal      = (frame * vec4(vnormal, 0)).xyz;
  gl_Position = projection * view * vec4(position, 1);
})";

const char* frag = R"(
#version 330
out vec4 result;
in vec3 position; in vec3 normal;

uniform mat4 frame; uniform mat4 view;
uniform mat4                     projection;

void main() {
  vec3 color = vec3(normal.y);
  color      = color * 0.5 + vec3(0.5);
  result     = vec4(pow(color, vec3(1 / 2.2)), 1);
})";

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

inline void mesh_viewer(const ioshape& mesh, const vec2i& viewport,
    const vec4f& background = {0, 0, 0, 1}) {
  // Init window.
  auto win = opengl_window();
  init_glwindow(win, viewport, "mesh viewer", nullptr);

  // Init shape.
  auto shape = opengl_shape{};
  init_glshape(shape);
  auto& normals = mesh.normals.empty()
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
  auto shader = opengl_program{};
  init_glprogram(shader, vert, frag);

  // Init camera.
  auto camera = make_lookat_camera(
      direction * box_size + box_center, box_center);

  // Draw.
  vec2f mouse, last_mouse;
  while (!should_glwindow_close(win)) {
    clear_glframebuffer(background);
    update_camera(camera.frame, camera.focus, mouse, last_mouse, win);

    bind_glprogram(shader);
    set_gluniform(shader, "frame", identity4x4f);
    set_gluniform(shader, "view", make_view_matrix(camera));
    set_gluniform(
        shader, "projection", make_projection_matrix(camera, viewport));
    draw_glshape(shape);

    swap_glbuffers(win);
    process_glevents(win, true);
  }

  delete_glwindow(win);
}
