#include "yocto_image.h"
#include "yocto_modelio.h"
#include "yocto_opengl.h"
#include "yocto_window.h"

#ifdef __APPLE__
#define GL_SILENCE_DEPRECATION
#endif
#include "ext/glad/glad.h"

using namespace yocto;
using namespace opengl;

struct mesh_viewer {
  vec2i            viewport          = {500, 500};
  vec3f            background        = {0, 0, 0};
  string           vertex_filename   = "shaders/mesh.vert";
  string           fragment_filename = "shaders/mesh.frag";
  opengl_callbacks callbacks         = {};
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

inline opengl_camera make_framing_camera(const vector<vec3f>& positions) {
  auto direction = vec3f{0, 1, 2};
  auto box       = bbox3f{};
  for (auto& p : positions) {
    expand(box, p);
  }
  auto box_center = center(box);
  auto box_size   = max(size(box));
  return make_lookat_camera(direction * box_size + box_center, box_center);
}

inline void run(const mesh_viewer& viewer, const ioshape& mesh) {
  // Init window.
  auto win      = opengl_window();
  win.callbacks = std::move(viewer.callbacks);
  init_glwindow(win, viewer.viewport, "mesh viewer");

  // Init shape.
  auto normals = mesh.normals.empty()
                     ? compute_normals(mesh.triangles, mesh.positions)
                     : mesh.normals;
  auto shape = make_glmesh(mesh.triangles, mesh.positions, normals);

  // Init camera.
  auto camera = make_framing_camera(mesh.positions);

  // Init shader.
  auto shader = create_glprogram(
      viewer.vertex_filename, viewer.fragment_filename, true);
  auto quad        = make_glquad();
  auto quad_shader = create_glprogram(
      "shaders/quad.vert", "shaders/quad-texture.frag", true);

  auto target = make_glrender_target(
      2 * viewer.viewport, false, true, true, true);
  // auto image  = load_image("/Users/nazzaro/Desktop/img.png");
  // init_gltexture(target.texture, image, true, true, true);

  win.callbacks.draw = [&](opengl_window& win, const opengl_input&) {
    auto view       = make_view_matrix(camera);
    auto projection = make_projection_matrix(camera, viewer.viewport);

    // clang-format off
    bind_glrender_target(target);
    glClearColor(0.1f, 0.1f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    draw_glshape(shape, shader,
      opengl_uniform{"color", vec3f{1, 1, 1}},
      opengl_uniform{"frame", identity4x4f},
      opengl_uniform{"view", view},
      opengl_uniform{"projection", projection}
    );
    // clang-format on

    unbind_glrender_target();
    bind_glprogram(quad_shader);
    set_gluniform_texture(quad_shader, "color_tex", target.texture, 0);
    set_gluniform(quad_shader, "color", vec3f{1, 1, 1});
    draw_glshape(quad);
  };

  // Draw.
  do {
    clear_glframebuffer(vec4f(viewer.background, 1));
    update_camera(camera.frame, camera.focus, win);
    win.draw();
  } while (!draw_loop(win));

  delete_glwindow(win);
}
