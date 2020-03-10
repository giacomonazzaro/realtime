#ifndef YOCTO_WINDOW
#define YOCTO_WINDOW

#include <functional>
using std::function;

#include "yocto_common.h"
#include "yocto_math.h"

// forward declaration
struct GLFWwindow;

namespace opengl {
using namespace yocto;

enum struct Key : int {
  // For printable keys, just use the constructor, like Key('*')
  // For chars, always use upper case!

  // Function keys
  escape        = 256,
  enter         = 257,
  tab           = 258,
  backspace     = 259,
  insert        = 260,
  _delete       = 261,
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

// Input state
struct Input {
  bool     mouse_left           = false;  // left button
  bool     mouse_right          = false;  // right button
  bool     mouse_middle         = false;  // middle button
  vec2f    mouse_pos            = {};     // position excluding widgets
  vec2f    mouse_last           = {};  // last mouse position excluding widgets
  vec2f    mouse_delta          = {};  // last mouse delta excluding widgets
  bool     modifier_alt         = false;         // alt modifier
  bool     modifier_ctrl        = false;         // ctrl modifier
  bool     modifier_shift       = false;         // shift modifier
  bool     widgets_active       = false;         // widgets are active
  uint64_t clock_now            = 0;             // clock now
  uint64_t clock_last           = 0;             // clock last
  double   time_now             = 0;             // time now
  double   time_delta           = 0;             // time delta
  vec2i    window_size          = {0, 0};        // window size
  float    window_aspect        = 0;             // window aspect ratio
  vec4i    framebuffer_viewport = {0, 0, 0, 0};  // framebuffer viewport
  float    framebuffer_aspect   = 0;             // framebuffer aspect ratio
};

struct Window;

struct Callbacks {
  // Called after opengl is initialized
  function<void(Window&, const Input&)> init;

  // Called at each frame
  function<void(Window&, const Input&)> draw;

  // Called when the contents of a window is damaged and needs to be refreshed
  function<void(Window&, const Input&)> refresh;

  // Called after draw for widgets
  function<void(Window&, const Input&)> widgets;

  // Called when drag and dropping files onto window.
  function<void(Window&, const Input&, const vector<string>&)> drop;

  // Called when a key is pressed or release.
  function<void(Window&, const Input&, int key, bool pressed)> key;

  // Called when a mouse button is pressed or release.
  function<void(Window&, const Input&, bool left, bool pressed)> click;

  // Called when scroll is preformed.
  function<void(Window&, const Input&, float amount)> scroll;
};

// OpenGL window wrapper
struct Window {
  GLFWwindow* glfw          = nullptr;
  string      title         = "";
  Callbacks   callbacks     = {};
  int         widgets_width = 0;
  bool        widgets_left  = true;
  Input       input         = {};
  vec4f       background    = {0.15f, 0.15f, 0.15f, 1.0f};

  // clang-format off
  void init() { callbacks.init(*this, input); }
  void draw() { callbacks.draw(*this, input); }
  void refresh() { callbacks.refresh(*this, input); }
  void widgets() { callbacks.widgets(*this, input); }
  void drop(const vector<string>& names) { callbacks.drop(*this, input, names); }
  void key(int key, bool pressed) { callbacks.key(*this, input, key, pressed); }
  void click(bool left, bool pressed) { callbacks.click(*this,  input, left, pressed); }
  void scroll(float amount) { callbacks.scroll(*this, input, amount); }
  // clang-format on
};

void init_window(Window& win, const vec2i& size, const string& title);
void delete_window(Window& win);

bool should_window_close(const Window& win);
void set_window_close(const Window& win, bool close = true);

vec2f get_mouse_pos_normalized(const Window& win);
bool  is_key_pressed(const Window& win, Key key);

void process_events(Window& win, bool wait = false);
void swap_buffers(const Window& win);
bool draw_loop(Window& win, bool wait = false);

void update_camera(frame3f& frame, float& focus, const Window& win);

// -----------------------------------------------------------------------------
// OPENGL WIDGETS
// -----------------------------------------------------------------------------

bool begin_header(Window& win, const char* title);
void end_header(Window& win);

void gui_label(Window& win, const char* lbl, const std::string& text);

void gui_separator(Window& win);
void continue_line(Window& win);

bool gui_button(Window& win, const char* lbl, bool enabled = true);

bool gui_textinput(Window& win, const char* lbl, std::string& value);

bool gui_slider(
    Window& win, const char* lbl, float& value, float min, float max);
bool gui_slider(
    Window& win, const char* lbl, vec2f& value, float min, float max);
bool gui_slider(
    Window& win, const char* lbl, vec3f& value, float min, float max);
bool gui_slider(
    Window& win, const char* lbl, vec4f& value, float min, float max);

bool gui_slider(Window& win, const char* lbl, int& value, int min, int max);
bool gui_slider(Window& win, const char* lbl, vec2i& value, int min, int max);
bool gui_slider(Window& win, const char* lbl, vec3i& value, int min, int max);
bool gui_slider(Window& win, const char* lbl, vec4i& value, int min, int max);

bool gui_dragger(Window& win, const char* lbl, float& value, float speed = 1.0f,
    float min = 0.0f, float max = 0.0f);
bool gui_dragger(Window& win, const char* lbl, vec2f& value, float speed = 1.0f,
    float min = 0.0f, float max = 0.0f);
bool gui_dragger(Window& win, const char* lbl, vec3f& value, float speed = 1.0f,
    float min = 0.0f, float max = 0.0f);
bool gui_dragger(Window& win, const char* lbl, vec4f& value, float speed = 1.0f,
    float min = 0.0f, float max = 0.0f);

bool gui_dragger(Window& win, const char* lbl, int& value, float speed = 1,
    int min = 0, int max = 0);
bool gui_dragger(Window& win, const char* lbl, vec2i& value, float speed = 1,
    int min = 0, int max = 0);
bool gui_dragger(Window& win, const char* lbl, vec3i& value, float speed = 1,
    int min = 0, int max = 0);
bool gui_dragger(Window& win, const char* lbl, vec4i& value, float speed = 1,
    int min = 0, int max = 0);

bool gui_checkbox(Window& win, const char* lbl, bool& value);

bool gui_coloredit(Window& win, const char* lbl, vec3f& value);
bool gui_coloredit(Window& win, const char* lbl, vec4f& value);

bool gui_hdrcoloredit(Window& win, const char* lbl, vec3f& value);
bool gui_hdrcoloredit(Window& win, const char* lbl, vec4f& value);

bool gui_combobox(Window& win, const char* lbl, int& idx,
    const std::vector<std::string>& labels);
bool gui_combobox(Window& win, const char* lbl, std::string& value,
    const std::vector<std::string>& labels);
bool gui_combobox(Window& win, const char* lbl, int& idx, int num,
    const std::function<std::string(int)>& labels, bool include_null = false);

template <typename T>
inline bool gui_combobox(Window& win, const char* lbl, T*& value,
    const std::vector<T*>& vals, bool include_null = false) {
  auto idx = -1;
  for (auto pos = 0; pos < vals.size(); pos++)
    if (vals[pos] == value) idx = pos;
  auto edited = gui_combobox(
      win, lbl, idx, (int)vals.size(), [&](int idx) { return vals[idx]->name; },
      include_null);
  if (edited) {
    value = idx >= 0 ? vals[idx] : nullptr;
  }
  return edited;
}

template <typename T>
inline bool gui_combobox(Window& win, const char* lbl, T*& value,
    const std::vector<T*>& vals, const std::vector<std::string>& labels,
    bool include_null = false) {
  auto idx = -1;
  for (auto pos = 0; pos < vals.size(); pos++)
    if (vals[pos] == value) idx = pos;
  auto edited = gui_combobox(
      win, lbl, idx, (int)vals.size(), [&](int idx) { return labels[idx]; },
      include_null);
  if (edited) {
    value = idx >= 0 ? vals[idx] : nullptr;
  }
  return edited;
}

void gui_progressbar(Window& win, const char* lbl, float fraction);

void gui_histogram(
    Window& win, const char* lbl, const std::vector<float>& values);
void gui_histogram(
    Window& win, const char* lbl, const std::vector<vec2f>& values);
void gui_histogram(
    Window& win, const char* lbl, const std::vector<vec3f>& values);
void gui_histogram(
    Window& win, const char* lbl, const std::vector<vec4f>& values);

bool gui_messages(Window& win);
void push_message(Window& win, const std::string& message);
bool gui_filedialog(Window& win, const char* lbl, std::string& path, bool save,
    const std::string& dirname, const std::string& filename,
    const std::string& filter);
bool gui_filedialog_button(Window& win, const char* button_lbl,
    bool button_active, const char* lbl, std::string& path, bool save,
    const std::string& dirname, const std::string& filename,
    const std::string& filter);

void log_info(Window& win, const std::string& msg);
void log_error(Window& win, const std::string& msg);
void clear_log(Window& win);
void gui_log(Window& win);

}  // namespace opengl

#endif
