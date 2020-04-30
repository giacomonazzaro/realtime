#ifndef _REALTIME_GPU_
#define _REALTIME_GPU_

// -----------------------------------------------------------------------------
// INCLUDES
// -----------------------------------------------------------------------------

#include <graphics/image.h>
#include <graphics/math.h>

#include <functional>   // std::function
#include <type_traits>  // std::is_floating_point


// -----------------------------------------------------------------------------
// LOW-LEVEL OPENGL OBJECTS
// -----------------------------------------------------------------------------
// namespace yocto {
namespace opengl {
using namespace yocto;

bool init_opengl();

// OpenGL shader
struct Shader {
  string vertex_code;
  string fragment_code;
  string vertex_filename;
  string fragment_filename;
  uint   shader_id   = 0;
  uint   vertex_id   = 0;
  uint   fragment_id = 0;
         operator uint() const { return shader_id; }
};

// OpenGL texture
struct Texture {
  uint  id       = 0;
  vec2i size     = {0, 0};
  bool  mipmap   = false;
  bool  linear   = false;
  bool  is_srgb  = false;
  bool  is_float = false;
        operator uint() const { return id; }
};

// OpenGL array buffer
struct Arraybuffer {
  uint id        = 0;
  int  num       = 0;
  int  elem_size = 0;
  bool is_index  = false;
       operator bool() const { return (bool)id; }
};

// OpenGL shape
struct Shape {
  vector<Arraybuffer> vertex_attributes = {};
  Arraybuffer         elements          = {};
  uint                id                = 0;

  enum struct type { points, lines, triangles };
  type type = type::triangles;
};

template <typename Type>
struct Uniform {
  const char* name;
  Type        value;
  Uniform(const char* n, const Type& v) : name(n), value(v) {}
};

// OpenGL image data
struct Image {
  Texture texture = {};
  Shader  shader  = {};
  Shape   shape   = {};

  vec2i size() const { return texture.size; }
};

// OpenGL image drawing params
struct draw_image_params {
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
void update_image(Image& glimage, const image<vec4f>& img, bool linear = false,
    bool mipmap = false);
void update_image(Image& glimage, const image<vec4b>& img, bool linear = false,
    bool mipmap = false);

// update the image data for a small region
void update_image_region(
    Image& glimage, const image<vec4f>& img, const image_region& region);
void update_image_region(
    Image& glimage, const image<vec4b>& img, const image_region& region);

// draw image
void draw_image(Image& glimage, const draw_image_params& params);

// Opengl caemra
struct Camera {
  frame3f frame = identity3x4f;
  float   lens  = 0.050;
  float   film  = 0.036;
  float   near  = 0.001;
  float   far   = 10000;
  float   focus = flt_max;
};

// Opengl material
struct Material {
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
struct Instance {
  frame3f frame       = identity3x4f;
  int     shape       = 0;
  int     material    = 0;
  bool    highlighted = false;
};

// Opengl light
struct Light {
  vec3f position = zero3f;
  vec3f emission = zero3f;
  int   type     = 0;
};

// Opengl scene
struct Scene {
  vector<Camera>   cameras   = {};
  vector<Instance> instances = {};
  vector<Shape>    shapes    = {};
  vector<Material> materials = {};
  vector<Texture>  textures  = {};
  vector<Light>    lights    = {};
  Shader           shader    = {};
};

// Draw options
struct draw_scene_params {
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
void make_scene(Scene& scene);

// Draw an OpenGL scene
void draw_scene(
    Scene& state, const vec4i& viewport, const draw_scene_params& params);

void check_error();

void clear_framebuffer(
    const vec4f& color = {0, 0, 0, 0}, bool clear_depth = true);

void set_viewport(const vec4i& viewport);

void set_wireframe(bool enabled);
void set_blending(bool enabled);
void set_point_size(int size);

void load_shader(Shader& shader, const string& vertex_filename,
    const string& fragment_filename);
void reload_shader(Shader& shader);
bool init_shader(Shader& shader, const char* vertex, const char* fragment,
    bool abort_on_error = false);
bool init_shader(Shader& shader, bool abort_on_error = false);

Shader create_shader(const string& vertex_filename,
    const string& fragment_filename, bool abort_on_error = false);

void delete_shader(Shader& shader);

void bind_shader(const Shader& shader);
void unbind_shader();

void init_texture(Texture& texture, const vec2i& size, bool as_float,
    bool as_srgb, bool linear, bool mipmap);

void update_texture(Texture& texture, const image<vec4f>& img, bool mipmap);
void update_texture_region(Texture& texture, const image<vec4f>& img,
    const image_region& region, bool mipmap);

inline void init_texture(Texture& texture, const image<vec4f>& img,
    bool as_float, bool linear, bool mipmap) {
  init_texture(texture, img.size(), as_float, false, linear, mipmap);
  update_texture(texture, img, mipmap);
}

void init_texture(Texture& texture, const image<vec4b>& img, bool as_srgb,
    bool linear, bool mipmap);
void update_texture(Texture& texture, const image<vec4b>& img, bool mipmap);
void update_texture_region(Texture& texture, const image<vec4b>& img,
    const image_region& region, bool mipmap);

inline void init_texture(Texture& texture, const image<vec4b>& img,
    bool as_srgb, bool linear, bool mipmap) {
  init_texture(texture, img.size(), false, as_srgb, linear, mipmap);
  update_texture(texture, img, mipmap);
}

void delete_texture(Texture& texture);

void init_shape(Shape& shape);
void delete_shape(Shape& shape);
void bind_shape(const Shape& shape);

void init_arraybuffer(Arraybuffer& buffer, const void* data, bool dynamic);
void init_arraybuffer(
    Arraybuffer& buffer, const vector<float>& array, bool dynamic);

template <typename T>
void init_arraybuffer(
    Arraybuffer& buffer, const vector<T>& array, bool dynamic = false) {
  buffer           = Arraybuffer{};
  buffer.num       = size(array);
  buffer.elem_size = sizeof(T);
  if constexpr (std::is_floating_point<T>::value) {
    buffer.is_index = false;
  } else {
    buffer.is_index = !std::is_floating_point<typeof(array[0][0])>::value;
  }
  init_arraybuffer(buffer, array.data(), dynamic);
}
void delete_arraybuffer(Arraybuffer& buffer);

int get_uniform_location(const Shader& shader, const char* name);

void set_uniform(int location, int value);
void set_uniform(int location, const vec2i& value);
void set_uniform(int location, const vec3i& value);
void set_uniform(int location, const vec4i& value);
void set_uniform(int location, float value);
void set_uniform(int location, const vec2f& value);
void set_uniform(int location, const vec3f& value);
void set_uniform(int location, const vec4f& value);
void set_uniform(int location, const mat2f& value);
void set_uniform(int location, const mat3f& value);
void set_uniform(int location, const mat4f& value);
void set_uniform(int location, const frame3f& value);

template <typename T>
inline void set_uniform(
    const Shader& shader, const char* name, const T& value) {
  set_uniform(get_uniform_location(shader, name), value);
}

void set_uniform(int location, const float* value, int num_values);
void set_uniform(int location, const vec3f* value, int num_values);

template <typename T>
inline void set_uniform(
    const Shader& shader, const char* name, const T* values, int num_values) {
  set_uniform(get_uniform_location(shader, name), values, num_values);
}

void set_uniform_texture(int location, const Texture& texture, int unit);
void set_uniform_texture(
    const Shader& shader, const char* name, const Texture& texture, int unit);
void set_uniform_texture(
    int location, int locatiom_on, const Texture& texture, int unit);
void set_uniform_texture(const Shader& shader, const char* name,
    const char* name_on, const Texture& texture, int unit);

int get_vertexattrib_location(const Shader& shader, const char* name);

void set_vertexattrib(int location, const Arraybuffer& buffer, int elem_size);
void set_vertexattrib(int location, const Arraybuffer& buffer, float value);
void set_vertexattrib(
    int location, const Arraybuffer& buffer, const vec2f& value);
void set_vertexattrib(
    int location, const Arraybuffer& buffer, const vec3f& value);
void set_vertexattrib(
    int location, const Arraybuffer& buffer, const vec4f& value);

template <typename T>
inline void set_vertexattrib(const Shader& shader, const char* name,
    const Arraybuffer& buffer, const T& value) {
  set_vertexattrib(get_vertexattrib_location(shader, name), buffer, value);
}

void draw_points(const Arraybuffer& buffer);
void draw_lines(const Arraybuffer& buffer);
void draw_triangles(const Arraybuffer& buffer);
void draw_point_strip(const Arraybuffer& buffer);
void draw_line_strip(const Arraybuffer& buffer);
void draw_triangle_strip(const Arraybuffer& buffer);

template <typename T>
int set_vertex_attribute(Shape& shape, int index, const vector<T>& data) {
  assert(index < shape.vertex_attributes.size());
  assert(shape.vertex_attributes[index].num == data.size());
  bind_shape(shape);
  // @Speed: update instead of delete
  delete_arraybuffer(shape.vertex_attributes[index]);
  init_arraybuffer(shape.vertex_attributes[index], data);
  int elem_size = sizeof(T) / sizeof(float);
  set_vertexattrib(index, shape.vertex_attributes[index], elem_size);
  return index;
}

template <typename T>
int add_vertex_attribute(Shape& shape, const vector<T>& data) {
  assert(shape.vertex_attributes.empty() ||
         shape.vertex_attributes[0].num == data.size());
  bind_shape(shape);
  int index = shape.vertex_attributes.size();
  shape.vertex_attributes.push_back({});
  init_arraybuffer(shape.vertex_attributes.back(), data);
  int elem_size = sizeof(T) / sizeof(float);
  set_vertexattrib(index, shape.vertex_attributes.back(), elem_size);
  return index;
}

template <typename T>
void init_elements(Shape& shape, const vector<T>& data) {
  bind_shape(shape);
  check_error();
  init_arraybuffer(shape.elements, data);
  int elem_size = sizeof(T) / sizeof(int);
  if (elem_size == 1) shape.type = Shape::type::points;
  if (elem_size == 2) shape.type = Shape::type::lines;
  if (elem_size == 3) shape.type = Shape::type::triangles;
  check_error();
}

void init_shape(Shape& shape);
void draw_shape(const Shape& shape);
void delete_shape(Shape& glshape);

Shape make_points(const vector<vec3f>& positions);
Shape make_polyline(const vector<vec3f>& position,
    const vector<vec3f>& normals = {}, float eps = 0.01);
Shape make_quad();
Shape make_regular_polygon(int num_sides);
Shape make_mesh(const vector<vec3i>& triangles, const vector<vec3f>& positions,
    const vector<vec3f>& normals);
Shape make_vector_field(const vector<vec3f>& vector_field,
    const vector<vec3f>& from, float scale = 0.01);
Shape make_vector_field(const vector<vec3f>& vector_field,
    const vector<vec3i>& triangles, const vector<vec3f>& positions,
    float scale = 0.001);

Camera make_lookat_camera(
    const vec3f& from, const vec3f& to, const vec3f& up = {0, 1, 0});

mat4f make_view_matrix(const Camera& camera);
mat4f make_projection_matrix(const Camera& camera, const vec2i& viewport,
    float near = 0.01, float far = 10000);

template <typename Type>
void set_uniform(const Shader& shader, const Uniform<Type>& u) {
  set_uniform(shader, u.name, u.value);
}

template <typename Type, typename... Args>
void set_uniform(const Shader& shader, const Uniform<Type>& u,
    const Uniform<Args>&... args) {
  set_uniform(shader, u);
  set_uniform(shader, args...);
}

template <typename... Args>
void draw_shape(
    const Shape& shape, const Shader& shader, const Uniform<Args>&... args) {
  bind_shader(shader);
  set_uniform(shader, args...);
  draw_shape(shape);
}

struct Rendertarget {
  Texture texture;
  uint    frame_buffer;
  uint    render_buffer;
};

Rendertarget make_render_target(
    const vec2i& size, bool as_float, bool as_srgb, bool linear, bool mipmap);
void bind_render_target(const Rendertarget& target);
void unbind_render_target();

// template <int N>
// Render_target make_render_targets(array<N, Texture>&
// textures);

}  // namespace opengl

#endif
