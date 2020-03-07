#ifndef _YOCTO_OPENGL_
#define _YOCTO_OPENGL_

// -----------------------------------------------------------------------------
// INCLUDES
// -----------------------------------------------------------------------------

#include <functional>

#include "yocto_image.h"
#include "yocto_math.h"

// -----------------------------------------------------------------------------
// LOW-LEVEL OPENGL OBJECTS
// -----------------------------------------------------------------------------
// namespace yocto {
namespace opengl {
using namespace yocto;

// OpenGL program
struct opengl_program {
  string vertex_code;
  string fragment_code;
  string vertex_filename;
  string fragment_filename;
  uint   program_id         = 0;
  uint   vertex_shader_id   = 0;
  uint   fragment_shader_id = 0;
         operator bool() const { return (bool)program_id; }
};

// OpenGL texture
struct opengl_texture {
  uint  texture_id = 0;
  vec2i size       = {0, 0};
  bool  mipmap     = false;
  bool  linear     = false;
  bool  is_srgb    = false;
  bool  is_float   = false;
        operator bool() const { return (bool)texture_id; }
};

// OpenGL vertex buffer
struct opengl_arraybuffer {
  uint buffer_id = 0;
  int  num       = 0;
  int  elem_size = 0;
       operator bool() const { return (bool)buffer_id; }
};

// OpenGL element buffer
struct opengl_elementbuffer {
  uint buffer_id = 0;
  int  num       = 0;
  int  elem_size = 0;
       operator bool() const { return (bool)buffer_id; }
};

// }  // namespace yocto

// -----------------------------------------------------------------------------
// HIGH-LEVEL OPENGL IMAGE DRAWING
// -----------------------------------------------------------------------------
// namespace yocto {

// OpenGL shape
struct opengl_shape {
  vector<opengl_arraybuffer> vertex_attributes = {};
  opengl_elementbuffer       elements          = {};
  uint                       vao               = 0;

  enum struct type { points, lines, triangles };
  type type = type::triangles;
};

// OpenGL image data
struct opengl_image {
  opengl_texture texture = {};
  opengl_program program = {};
  opengl_shape   shape   = {};

  vec2i size() const { return texture.size; }
        operator bool() const { return (bool)texture; }
};

// OpenGL image drawing params
struct draw_glimage_params {
  vec2i window      = {512, 512};
  vec4i framebuffer = {0, 0, 512, 512};
  vec2f center      = {0, 0};
  float scale       = 1;
  bool  fit         = true;
  bool  checker     = true;
  float border_size = 2;
  vec4f background  = {0.15f, 0.15f, 0.15f, 1.0f};
};

// update image data
void update_glimage(opengl_image& glimage, const image<vec4f>& img,
    bool linear = false, bool mipmap = false);
void update_glimage(opengl_image& glimage, const image<vec4b>& img,
    bool linear = false, bool mipmap = false);

// update the image data for a small region
void update_glimage_region(
    opengl_image& glimage, const image<vec4f>& img, const image_region& region);
void update_glimage_region(
    opengl_image& glimage, const image<vec4b>& img, const image_region& region);

// draw image
void draw_glimage(opengl_image& glimage, const draw_glimage_params& params);

// }  // namespace yocto

// -----------------------------------------------------------------------------
// HIGH-LEVEL OPENGL SCENE RENDERING
// -----------------------------------------------------------------------------
// namespace yocto {

// Opengl caemra
struct opengl_camera {
  frame3f frame  = identity3x4f;
  float   lens   = 0.050;
  float   asepct = 1;
  float   film   = 0.036;
  float   near   = 0.001;
  float   far    = 10000;
  float   focus  = flt_max;
};

// Opengl material
struct opengl_material {
  vec3f emission      = zero3f;
  vec3f diffuse       = zero3f;
  vec3f specular      = zero3f;
  float metallic      = 0;
  float roughness     = 0;
  float opacity       = 1;
  int   emission_map  = -1;
  int   diffuse_map   = -1;
  int   specular_map  = -1;
  int   metallic_map  = -1;
  int   roughness_map = -1;
  int   normal_map    = -1;
  bool  gltf_textures = false;
};

// Opengl instance group
struct opengl_instance {
  frame3f frame       = identity3x4f;
  int     shape       = 0;
  int     material    = 0;
  bool    highlighted = false;
};

// Opengl light
struct opengl_light {
  vec3f position = zero3f;
  vec3f emission = zero3f;
  int   type     = 0;
};

// Opengl scene
struct opengl_scene {
  vector<opengl_camera>   cameras   = {};
  vector<opengl_instance> instances = {};
  vector<opengl_shape>    shapes    = {};
  vector<opengl_material> materials = {};
  vector<opengl_texture>  textures  = {};
  vector<opengl_light>    lights    = {};
  opengl_program          program   = {};
};

// Draw options
struct draw_glscene_params {
  int   camera           = 0;
  int   resolution       = 1280;
  bool  wireframe        = false;
  bool  edges            = false;
  float edge_offset      = 0.01f;
  bool  eyelight         = false;
  float exposure         = 0;
  float gamma            = 2.2f;
  vec3f ambient          = {0, 0, 0};
  bool  double_sided     = true;
  bool  non_rigid_frames = true;
  float near             = 0.01f;
  float far              = 10000.0f;
  vec4f background       = vec4f{0.15f, 0.15f, 0.15f, 1.0f};
};

// Initialize an OpenGL scene
void make_glscene(opengl_scene& scene);

// Draw an OpenGL scene
void draw_glscene(opengl_scene& state, const vec4i& viewport,
    const draw_glscene_params& params);

// }  // namespace yocto

// -----------------------------------------------------------------------------
// LOW-LEVEL OPENGL FUNCTIONS
// -----------------------------------------------------------------------------
// namespace yocto {

void check_glerror();

void clear_glframebuffer(const vec4f& color, bool clear_depth = true);

void set_glviewport(const vec4i& viewport);

void set_glwireframe(bool enabled);
void set_glblending(bool enabled);

void load_glprogram(opengl_program& program, const string& vertex_filename,
    const string& fragment_filename);
void reload_glprogram(opengl_program& program);
bool init_glprogram(opengl_program& program, const char* vertex,
    const char* fragment, bool abort_on_error = false);
bool init_glprogram(opengl_program& program, bool abort_on_error = false);

opengl_program create_glprogram(const string& vertex_filename,
    const string& fragment_filename, bool abort_on_error = false);

void delete_glprogram(opengl_program& program);

void bind_glprogram(const opengl_program& program);
void unbind_opengl_program();

void init_gltexture(opengl_texture& texture, const vec2i& size, bool as_float,
    bool as_srgb, bool linear, bool mipmap);

void update_gltexture(
    opengl_texture& texture, const image<vec4f>& img, bool mipmap);
void update_gltexture_region(opengl_texture& texture, const image<vec4f>& img,
    const image_region& region, bool mipmap);

inline void init_gltexture(opengl_texture& texture, const image<vec4f>& img,
    bool as_float, bool linear, bool mipmap) {
  init_gltexture(texture, img.size(), as_float, false, linear, mipmap);
  update_gltexture(texture, img, mipmap);
}

void init_gltexture(opengl_texture& texture, const image<vec4b>& img,
    bool as_srgb, bool linear, bool mipmap);
void update_gltexture(
    opengl_texture& texture, const image<vec4b>& img, bool mipmap);
void update_gltexture_region(opengl_texture& texture, const image<vec4b>& img,
    const image_region& region, bool mipmap);

inline void init_gltexture(opengl_texture& texture, const image<vec4b>& img,
    bool as_srgb, bool linear, bool mipmap) {
  init_gltexture(texture, img.size(), false, as_srgb, linear, mipmap);
  update_gltexture(texture, img, mipmap);
}

void delete_gltexture(opengl_texture& texture);

bool init_opengl_vertex_array_object(uint& vao);
bool delete_opengl_vertex_array_object(uint& vao);
bool bind_opengl_vertex_array_object(uint vao);

void init_glarraybuffer(opengl_arraybuffer& buffer, const vector<float>& data,
    bool dynamic = false);
void init_glarraybuffer(opengl_arraybuffer& buffer, const vector<vec2f>& data,
    bool dynamic = false);
void init_glarraybuffer(opengl_arraybuffer& buffer, const vector<vec3f>& data,
    bool dynamic = false);
void init_glarraybuffer(opengl_arraybuffer& buffer, const vector<vec4f>& data,
    bool dynamic = false);

void delete_glarraybuffer(opengl_arraybuffer& buffer);

void init_glelementbuffer(opengl_elementbuffer& buffer, const vector<int>& data,
    bool dynamic = false);
void init_glelementbuffer(opengl_elementbuffer& buffer,
    const vector<vec2i>& data, bool dynamic = false);
void init_glelementbuffer(opengl_elementbuffer& buffer,
    const vector<vec3i>& data, bool dynamic = false);

void delete_glelementbuffer(opengl_elementbuffer& buffer);

int get_gluniform_location(const opengl_program& program, const char* name);

void set_gluniform(int location, int value);
void set_gluniform(int location, const vec2i& value);
void set_gluniform(int location, const vec3i& value);
void set_gluniform(int location, const vec4i& value);
void set_gluniform(int location, float value);
void set_gluniform(int location, const vec2f& value);
void set_gluniform(int location, const vec3f& value);
void set_gluniform(int location, const vec4f& value);
void set_gluniform(int location, const mat2f& value);
// void set_gluniform(int location, const mat3f& value);
void set_gluniform(int location, const mat4f& value);
void set_gluniform(int location, const frame3f& value);

template <typename T>
inline void set_gluniform(
    const opengl_program& program, const char* name, const T& value) {
  set_gluniform(get_gluniform_location(program, name), value);
}

void set_gluniform(int location, const float* value, int num_values);
void set_gluniform(int location, const vec3f* value, int num_values);

template <typename T>
inline void set_gluniform(const opengl_program& program, const char* name,
    const T* values, int num_values) {
  set_gluniform(get_gluniform_location(program, name), values, num_values);
}

void set_gluniform_texture(
    int location, const opengl_texture& texture, int unit);
void set_gluniform_texture(const opengl_program& program, const char* name,
    const opengl_texture& texture, int unit);
void set_gluniform_texture(
    int location, int locatiom_on, const opengl_texture& texture, int unit);
void set_gluniform_texture(const opengl_program& program, const char* name,
    const char* name_on, const opengl_texture& texture, int unit);

int get_glvertexattrib_location(
    const opengl_program& program, const char* name);

void set_glvertexattrib(
    int location, const opengl_arraybuffer& buffer, int elem_size);
void set_glvertexattrib(
    int location, const opengl_arraybuffer& buffer, float value);
void set_glvertexattrib(
    int location, const opengl_arraybuffer& buffer, const vec2f& value);
void set_glvertexattrib(
    int location, const opengl_arraybuffer& buffer, const vec3f& value);
void set_glvertexattrib(
    int location, const opengl_arraybuffer& buffer, const vec4f& value);

template <typename T>
inline void set_glvertexattrib(const opengl_program& program, const char* name,
    const opengl_arraybuffer& buffer, const T& value) {
  set_glvertexattrib(get_glvertexattrib_location(program, name), buffer, value);
}

void draw_glpoints(const opengl_elementbuffer& buffer, int num);
void draw_glpoints(const opengl_arraybuffer& buffer, int num);
void draw_gllines(const opengl_elementbuffer& buffer, int num);
void draw_gllines(const opengl_arraybuffer& buffer, int num);
void draw_gltriangles(const opengl_elementbuffer& buffer, int num);
void draw_gltriangles(const opengl_arraybuffer& buffer, int num);

template <typename T>
void add_vertex_attribute(opengl_shape& shape, const vector<T>& data) {
  bind_opengl_vertex_array_object(shape.vao);
  int index = shape.vertex_attributes.size();
  shape.vertex_attributes.push_back({});
  init_glarraybuffer(shape.vertex_attributes.back(), data);
  int elem_size = sizeof(T) / sizeof(float);
  set_glvertexattrib(index, shape.vertex_attributes.back(), elem_size);
}

template <typename T>
void init_elements(opengl_shape& shape, const vector<T>& data) {
  bind_opengl_vertex_array_object(shape.vao);
  check_glerror();
  init_glelementbuffer(shape.elements, data);
  int elem_size = sizeof(T) / sizeof(int);
  if (elem_size == 1) shape.type = opengl_shape::type::points;
  if (elem_size == 2) shape.type = opengl_shape::type::lines;
  if (elem_size == 3) shape.type = opengl_shape::type::triangles;
  check_glerror();
}

void init_glshape(opengl_shape& shape);
void draw_glshape(const opengl_shape& shape);
void delete_glshape(opengl_shape& glshape);

opengl_shape make_glquad(opengl_shape& shape);
opengl_shape make_glpath(const vector<vec3f>& position,
    const vector<vec3f>& normals = {}, float eps = 0.01);
opengl_shape make_glvector_field(const vector<vec3f>& vector_field,
    const vector<vec3f>& from, float scale = 0.01);
opengl_shape make_glvector_field(const vector<vec3f>& vector_field,
    const vector<vec3i>& triangles, const vector<vec3f>& positions,
    float scale = 0.001);

opengl_camera make_lookat_camera(
    const vec3f& from, const vec3f& to, const vec3f& up = {0, 1, 0});

mat4f make_view_matrix(const opengl_camera& camera);
mat4f make_projection_matrix(const opengl_camera& camera, const vec2i& viewport,
    float near = 0.01, float far = 10000);

}  // namespace opengl

#endif
