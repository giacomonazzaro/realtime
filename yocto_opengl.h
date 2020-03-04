//
// Yocto/OpenGL: Utilities to use OpenGL 3, GLFW and ImGui.
//

//
// LICENSE:
//
// Copyright (c) 2016 -- 2019 Fabio Pellacini
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//
//

#ifndef _YOCTO_OPENGL_
#define _YOCTO_OPENGL_

// -----------------------------------------------------------------------------
// INCLUDES
// -----------------------------------------------------------------------------

#include <functional>

#include "yocto_image.h"
#include "yocto_math.h"

// forward declaration
struct GLFWwindow;

// -----------------------------------------------------------------------------
// LOW-LEVEL OPENGL OBJECTS
// -----------------------------------------------------------------------------
namespace yocto {

// OpenGL program
struct opengl_program {
  opengl_program() {}
  opengl_program(opengl_program&&);
  opengl_program& operator=(opengl_program&&);
  ~opengl_program();
  operator bool() const { return (bool)program_id; }

  uint program_id         = 0;
  uint vertex_shader_id   = 0;
  uint fragment_shader_id = 0;
};

// OpenGL texture
struct opengl_texture {
  opengl_texture() {}
  opengl_texture(opengl_texture&&);
  opengl_texture& operator=(opengl_texture&&);
  ~opengl_texture();
  operator bool() const { return (bool)texture_id; }

  uint  texture_id = 0;
  vec2i size       = {0, 0};
  bool  mipmap     = false;
  bool  linear     = false;
  bool  is_srgb    = false;
  bool  is_float   = false;
};

// OpenGL vertex buffer
struct opengl_arraybuffer {
  opengl_arraybuffer() {}
  opengl_arraybuffer(opengl_arraybuffer&&);
  opengl_arraybuffer& operator=(opengl_arraybuffer&&);
  ~opengl_arraybuffer();
  operator bool() const { return (bool)buffer_id; }

  uint buffer_id = 0;
  int  num       = 0;
  int  elem_size = 0;
};

// OpenGL element buffer
struct opengl_elementbuffer {
  opengl_elementbuffer() {}
  opengl_elementbuffer(opengl_elementbuffer&&);
  opengl_elementbuffer& operator=(opengl_elementbuffer&&);
  ~opengl_elementbuffer();
  operator bool() const { return (bool)buffer_id; }

  uint buffer_id = 0;
  int  num       = 0;
  int  elem_size = 0;
};

enum struct opengl_key : int {
  // For printable keys, just use the constructor, like opengl_key('*')
  // For chars, always use upper case!

  // Function keys
  escape    = 256,
  enter     = 257,
  tab       = 258,
  backspace = 259,
  insert    = 260,
  //  delete    = 261,
  right         = 262,
  left          = 263,
  down          = 264,
  up            = 265,
  page_up       = 266,
  page_down     = 267,
  home          = 268,
  end           = 269,
  caps_lock     = 280,
  scroll_lock   = 281,
  num_lock      = 282,
  print_screen  = 283,
  pause         = 284,
  f1            = 290,
  f2            = 291,
  f3            = 292,
  f4            = 293,
  f5            = 294,
  f6            = 295,
  f7            = 296,
  f8            = 297,
  f9            = 298,
  f10           = 299,
  f11           = 300,
  f12           = 301,
  f13           = 302,
  f14           = 303,
  f15           = 304,
  f16           = 305,
  f17           = 306,
  f18           = 307,
  f19           = 308,
  f20           = 309,
  f21           = 310,
  f22           = 311,
  f23           = 312,
  f24           = 313,
  f25           = 314,
  kp_0          = 320,
  kp_1          = 321,
  kp_2          = 322,
  kp_3          = 323,
  kp_4          = 324,
  kp_5          = 325,
  kp_6          = 326,
  kp_7          = 327,
  kp_8          = 328,
  kp_9          = 329,
  kp_decimal    = 330,
  kp_divide     = 331,
  kp_multiply   = 332,
  kp_subtract   = 333,
  kp_add        = 334,
  kp_enter      = 335,
  kp_equal      = 336,
  left_shift    = 340,
  left_control  = 341,
  left_alt      = 342,
  left_super    = 343,
  right_shift   = 344,
  right_control = 345,
  right_alt     = 346,
  right_super   = 347,
  menu          = 348,
  world_1       = 161,  //  non-us #1
  world_2       = 162   //  non-us #2
};

}  // namespace yocto

// -----------------------------------------------------------------------------
// HIGH-LEVEL OPENGL IMAGE DRAWING
// -----------------------------------------------------------------------------
namespace yocto {

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
  vec2i          size() const { return texture.size; }
                 operator bool() const { return (bool)texture; }
  opengl_texture texture = {};
  opengl_program program = {};
  opengl_shape   shape   = {};
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

}  // namespace yocto

// -----------------------------------------------------------------------------
// HIGH-LEVEL OPENGL SCENE RENDERING
// -----------------------------------------------------------------------------
namespace yocto {

// Opengl caemra
struct opengl_camera {
  frame3f frame  = identity3x4f;
  float   lens   = 0.050;
  float   asepct = 1;
  float   film   = 0.036;
  float   near   = 0.001;
  float   far    = 10000;
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

}  // namespace yocto

// -----------------------------------------------------------------------------
// LOW-LEVEL OPENGL FUNCTIONS
// -----------------------------------------------------------------------------
namespace yocto {

void check_glerror();

void clear_glframebuffer(const vec4f& color, bool clear_depth = true);

void set_glviewport(const vec4i& viewport);

void set_glwireframe(bool enabled);
void set_glblending(bool enabled);

bool init_glprogram(opengl_program& program, const char* vertex,
    const char* fragment, bool abort_on_error = false);

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

}  // namespace yocto

// -----------------------------------------------------------------------------
// OPENGL WINDOW
// -----------------------------------------------------------------------------
namespace yocto {

struct opengl_window;
using drop_glcallback =
    std::function<void(const opengl_window&, const vector<string>&)>;
using key_glcallback =
    std::function<void(const opengl_window&, opengl_key, bool)>;
// is_left_click, is_pressing
using click_glcallback  = std::function<void(const opengl_window&, bool, bool)>;
using scroll_glcallback = std::function<void(const opengl_window&, float)>;

struct opengl_window {
  GLFWwindow*       win           = nullptr;
  void*             user_ptr      = nullptr;
  drop_glcallback   drop_cb       = {};
  key_glcallback    key_cb        = {};
  click_glcallback  click_cb      = {};
  scroll_glcallback scroll_cb     = {};
  int               widgets_width = 0;
  bool              widgets_left  = true;
};

void init_glwindow(opengl_window& win, const vec2i& size, const string& title,
    void* user_pointer);
void delete_glwindow(opengl_window& win);

void set_drop_glcallback(opengl_window& win, drop_glcallback drop_cb);
void set_key_glcallback(opengl_window& win, key_glcallback cb);
void set_click_glcallback(opengl_window& win, click_glcallback cb);
void set_scroll_glcallback(opengl_window& win, scroll_glcallback cb);

void* get_gluser_pointer(const opengl_window& win);

vec2i get_glwindow_size(const opengl_window& win);
float get_glwindow_aspect_ratio(const opengl_window& win);
vec2i get_glframebuffer_size(const opengl_window& win);
vec4i get_glframebuffer_viewport(const opengl_window& win);

bool should_glwindow_close(const opengl_window& win);
void set_glwindow_close(const opengl_window& win, bool close);

vec2f get_glmouse_pos(const opengl_window& win);
vec2f get_glmouse_pos_normalized(const opengl_window& win);

bool get_glmouse_left(const opengl_window& win);
bool get_glmouse_right(const opengl_window& win);
bool get_glalt_key(const opengl_window& win);
bool get_glshift_key(const opengl_window& win);
bool is_key_pressed(const opengl_window& win, opengl_key key);

void process_glevents(const opengl_window& win, bool wait = false);
void swap_glbuffers(const opengl_window& win);

}  // namespace yocto

// -----------------------------------------------------------------------------
// OPENGL WIDGETS
// -----------------------------------------------------------------------------
namespace yocto {

void init_glwidgets(opengl_window& win, int width = 320, bool left = true);
bool get_glwidgets_active(const opengl_window& win);

void begin_glwidgets(const opengl_window& win);
void begin_glwidgets(
    const opengl_window& win, const vec2f& position, const vec2f& size);
void end_glwidgets(const opengl_window& win);

bool begin_glwidgets_window(const opengl_window& win, const char* title);

bool begin_glheader(const opengl_window& win, const char* title);
void end_glheader(const opengl_window& win);

void open_glmodal(const opengl_window& win, const char* lbl);
void clear_glmodal(const opengl_window& win);
bool begin_glmodal(const opengl_window& win, const char* lbl);
void end_glmodal(const opengl_window& win);
bool is_glmodal_open(const opengl_window& win, const char* lbl);

bool draw_glmessages(const opengl_window& win);
void push_glmessage(const string& message);
void push_glmessage(const opengl_window& win, const string& message);
bool draw_glfiledialog(const opengl_window& win, const char* lbl, string& path,
    bool save, const string& dirname, const string& filename,
    const string& filter);
bool draw_glfiledialog_button(const opengl_window& win, const char* button_lbl,
    bool button_active, const char* lbl, string& path, bool save,
    const string& dirname, const string& filename, const string& filter);

void draw_gllabel(
    const opengl_window& win, const char* lbl, const string& text);

bool begin_header_widget(const opengl_window& win, const char* label);
void end_header_widget(const opengl_window& win);

void draw_glseparator(const opengl_window& win);
void continue_glline(const opengl_window& win);

bool draw_glbutton(const opengl_window& win, const char* lbl);
bool draw_glbutton(const opengl_window& win, const char* lbl, bool enabled);

bool draw_gltextinput(const opengl_window& win, const char* lbl, string& value);
bool draw_glslider(const opengl_window& win, const char* lbl, float& value,
    float min, float max);
bool draw_glslider(const opengl_window& win, const char* lbl, vec2f& value,
    float min, float max);
bool draw_glslider(const opengl_window& win, const char* lbl, vec3f& value,
    float min, float max);
bool draw_glslider(const opengl_window& win, const char* lbl, vec4f& value,
    float min, float max);

bool draw_glslider(
    const opengl_window& win, const char* lbl, int& value, int min, int max);
bool draw_glslider(
    const opengl_window& win, const char* lbl, vec2i& value, int min, int max);
bool draw_glslider(
    const opengl_window& win, const char* lbl, vec3i& value, int min, int max);
bool draw_glslider(
    const opengl_window& win, const char* lbl, vec4i& value, int min, int max);

bool draw_gldragger(const opengl_window& win, const char* lbl, float& value,
    float speed = 1.0f, float min = 0.0f, float max = 0.0f);
bool draw_gldragger(const opengl_window& win, const char* lbl, vec2f& value,
    float speed = 1.0f, float min = 0.0f, float max = 0.0f);
bool draw_gldragger(const opengl_window& win, const char* lbl, vec3f& value,
    float speed = 1.0f, float min = 0.0f, float max = 0.0f);
bool draw_gldragger(const opengl_window& win, const char* lbl, vec4f& value,
    float speed = 1.0f, float min = 0.0f, float max = 0.0f);

bool draw_gldragger(const opengl_window& win, const char* lbl, int& value,
    float speed = 1, int min = 0, int max = 0);
bool draw_gldragger(const opengl_window& win, const char* lbl, vec2i& value,
    float speed = 1, int min = 0, int max = 0);
bool draw_gldragger(const opengl_window& win, const char* lbl, vec3i& value,
    float speed = 1, int min = 0, int max = 0);
bool draw_gldragger(const opengl_window& win, const char* lbl, vec4i& value,
    float speed = 1, int min = 0, int max = 0);

bool draw_glcheckbox(const opengl_window& win, const char* lbl, bool& value);

bool draw_glcoloredit(const opengl_window& win, const char* lbl, vec3f& value);
bool draw_glcoloredit(const opengl_window& win, const char* lbl, vec4f& value);

bool draw_glhdrcoloredit(
    const opengl_window& win, const char* lbl, vec3f& value);
bool draw_glhdrcoloredit(
    const opengl_window& win, const char* lbl, vec4f& value);

bool draw_glcombobox(const opengl_window& win, const char* lbl, int& idx,
    const vector<string>& labels);
bool draw_glcombobox(const opengl_window& win, const char* lbl, string& value,
    const vector<string>& labels);
bool draw_glcombobox(const opengl_window& win, const char* lbl, int& idx,
    int num, const std::function<const char*(int)>& labels,
    bool include_null = false);

template <typename T>
inline bool draw_glcombobox(const opengl_window& win, const char* lbl, int& idx,
    const vector<T>& vals, bool include_null = false) {
  return draw_glcombobox(
      win, lbl, idx, (int)vals.size(),
      [&](int idx) { return vals[idx].name.c_str(); }, include_null);
}

void draw_glhistogram(
    const opengl_window& win, const char* lbl, const vector<float>& values);
void draw_glhistogram(
    const opengl_window& win, const char* lbl, const vector<vec2f>& values);
void draw_glhistogram(
    const opengl_window& win, const char* lbl, const vector<vec3f>& values);
void draw_glhistogram(
    const opengl_window& win, const char* lbl, const vector<vec4f>& values);

void log_glinfo(const opengl_window& win, const string& msg);
void log_glerror(const opengl_window& win, const string& msg);
void clear_gllogs(const opengl_window& win);
void draw_gllog(const opengl_window& win);

inline vec2i get_framebuffer_size(const opengl_window& win);

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

void init_glquad(opengl_shape& shape);

}  // namespace yocto

#endif
