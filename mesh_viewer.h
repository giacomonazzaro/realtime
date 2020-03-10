#include "yocto_image.h"
#include "yocto_modelio.h"
#include "yocto_opengl.h"
#include "yocto_window.h"
using namespace yocto;
using namespace opengl;

struct mesh_viewer {
  vec2i     viewport          = {500, 500};
  vec3f     background        = {0, 0, 0};
  string    vertex_filename   = "shaders/mesh.vert";
  string    fragment_filename = "shaders/mesh.frag";
  Callbacks callbacks         = {};
};

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

#define bind(n, v) Uniform(n, v)

inline void run(const mesh_viewer& viewer, const ioshape& mesh) {
  // Init window.
  auto win      = Window();
  win.callbacks = std::move(viewer.callbacks);
  init_window(win, viewer.viewport, "mesh viewer");

  // Init shape.
  auto normals = mesh.normals.empty()
                     ? compute_normals(mesh.triangles, mesh.positions)
                     : mesh.normals;
  auto shape = make_mesh(mesh.triangles, mesh.positions, normals);

  // Init camera.
  auto camera = make_framing_camera(mesh.positions);

  // Init shader.
  auto shader = create_program(
      viewer.vertex_filename, viewer.fragment_filename, true);
  auto normal_shader = create_program(
      viewer.vertex_filename, "shaders/mesh_normals.frag", true);

  auto quad        = make_quad();
  auto quad_shader = create_program(
      "shaders/quad.vert", "shaders/quad-texture.frag", true);
  auto si           = win.framebuffer_size;
  auto color_buffer = make_render_target(si, false, false, true, true);
  auto color_normal = make_render_target(si, false, false, true, true);

  // auto image  = load_image("/Users/nazzaro/Desktop/img.png");
  // init_texture(target.texture, image, true, true, true);

  win.callbacks.draw = [&](Window& win) {
    update_camera(camera.frame, camera.focus, win);

    auto view       = make_view_matrix(camera);
    auto projection = make_projection_matrix(camera, viewer.viewport);

    // clang-format off
    bind_render_target(color_buffer);
    clear_framebuffer(vec4f(viewer.background, 1));
    draw_shape(shape, shader,
      bind("color", vec3f(1, 1, 1)),
      bind("frame", identity4x4f),
      bind("view", view),
      bind("projection", projection)
    );
    unbind_render_target();
    // clang-format on

    clear_framebuffer(vec4f(1, 1, 1, 1));
    bind_program(quad_shader);
    set_uniform_texture(quad_shader, "color_tex", color_buffer.texture, 0);
    draw_shape(quad, quad_shader, Uniform{"color", vec3f{1, 1, 1}});
  };

  // Draw.
  run_draw_loop(win);

  delete_window(win);
}
