#include <stdio.h>

#include <vector>
using namespace std;

#include "../yocto_modelio.h"
#include "../yocto_opengl.h"
#include "../yocto_window.h"
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

int main(int num_args, const char* args[]) {
  // Init window.
  auto win = Window();

  init_window(win, {300, 300}, "mesh viewer");

  auto mesh = load_shape(args[1]);

  // Init camera.
  auto camera = make_framing_camera(mesh.positions);

  // Init shape.
  auto normals = mesh.normals.empty()
                     ? compute_normals(mesh.triangles, mesh.positions)
                     : mesh.normals;
  auto shape = make_mesh(mesh.triangles, mesh.positions, normals);
  auto quad  = make_quad();

  // Init shader.
  auto color_shader = create_shader(
      "shaders/mesh.vert", "shaders/mesh_color.frag", true);
  auto normal_shader = create_shader(
      "shaders/mesh.vert", "shaders/mesh_normals.frag", true);
  auto quad_shader = create_shader(
      "shaders/quad.vert", "shaders/quad_shade.frag", true);

  auto framebuffer  = win.framebuffer_size;
  auto color_buffer = make_render_target(framebuffer, false, false, true, true);
  auto normal_buffer = make_render_target(
      framebuffer, false, false, true, true);

  auto draw = [&](Window& win) {
    update_camera(camera.frame, camera.focus, win);

    auto view       = make_view_matrix(camera);
    auto projection = make_projection_matrix(camera, win.size);

    // clang-format off
    bind_render_target(color_buffer);
    clear_framebuffer(vec4f(0, 0, 0, 1));
    draw_shape(shape, color_shader,
      Uniform("color", vec3f(1, 1, 1)),
      Uniform("frame", identity4x4f),
      Uniform("view", view),
      Uniform("projection", projection)
    );

    bind_render_target(normal_buffer);
    clear_framebuffer(vec4f(0, 0, 0, 1));
    draw_shape(shape, normal_shader,
      Uniform("color", vec3f(1, 1, 1)),
      Uniform("frame", identity4x4f),
      Uniform("view", view),
      Uniform("projection", projection)
    );
    // clang-format on

    unbind_render_target();
    clear_framebuffer(vec4f(1, 1, 1, 1));
    bind_shader(quad_shader);
    set_uniform_texture(quad_shader, "color_tex", color_buffer.texture, 0);
    set_uniform_texture(quad_shader, "normal_tex", normal_buffer.texture, 1);
    draw_shape(quad);
  };

  run_draw_loop(win, draw);
  delete_window(win);
}
