#ifndef YOCTO_WINDOW
#define YOCTO_WINDOW

#include <functional>

#include "yocto_common.h"
#include "yocto_math.h"

// forward declaration
struct GLFWwindow;

namespace yocto {

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
bool draw_loop(const opengl_window& win, bool wait = false);

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

}  // namespace yocto

#endif
