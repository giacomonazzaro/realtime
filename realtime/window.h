#ifndef _REALTIME_WINDOW_
#define _REALTIME_WINDOW_

#include <functional>
using std::function;

#include <graphics/common.h>
#include <graphics/math.h>

// Forward declaration
struct GLFWwindow;

namespace window {
using namespace yocto;

enum struct Key : int {
  // For printable keys, just use the constructor, like Key('*').
  // For letters, always use upper case, like Key('C').

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

// Data structure where all the inputs are stored between frames.
struct Input {
  bool     mouse_left        = false;  // left button
  bool     mouse_right       = false;  // right button
  bool     mouse_middle      = false;  // middle button
  vec2f    mouse_pos         = {};     // position excluding gui
  vec2f    mouse_last        = {};     // last mouse position excluding gui
  bool     modifier_alt      = false;  // alt modifier
  bool     modifier_ctrl     = false;  // ctrl modifier
  bool     modifier_shift    = false;  // shift modifier
  bool     is_window_focused = false;  // window is focused
  bool     is_gui_active     = false;  // gui is active
  uint64_t clock_now         = 0;      // clock now
  uint64_t clock_last        = 0;      // clock last
  double   time_now          = 0;      // time now
  double   time_delta        = 0;      // time delta
  int      frame             = 0;
};

// Data structure where the input of a joystick is stored.
struct Joystick {
  vec2f left_stick, right_stick;
  float left_trigger, right_trigger;
  bool  buttons[15];
  int   id = -1;

  // Handy interface for buttons. The mapping may be broken on some platform.
  // Using the recommanded mapping from GLFW gave wrong results with a PS4
  // Dualshock on a Mac. Change the implementation of these methods if needed.
  bool cross() const;
  bool circle() const;
  bool triangle() const;
  bool square() const;
  bool left_bumper() const;
  bool right_bumper() const;
  bool start() const;
  bool back() const;
  bool guide() const;
  bool A() const;
  bool B() const;
  bool X() const;
  bool Y() const;
};

// Forward declaration.
struct Window;

// Callbacks of a window, grouped in a struct for convenience.
struct Callbacks {
  // Called when a key is pressed or release.
  function<void(Window&, Key key, bool pressed)> key;

  // Called when a mouse button is pressed or release.
  function<void(Window&, bool left, bool pressed)> click;

  // Called when scroll is preformed.
  function<void(Window&, float amount)> scroll;

  // Called when drag and dropping files onto window.
  function<void(Window&, const vector<string>&)> drop;

  // Called when window gains or lose focus.
  function<void(Window&, int focused)> focus;
};

// Info to open and handle a new window within the OS.
struct Window {
  string           title     = "";
  GLFWwindow*      glfw      = nullptr;
  Input            input     = {};
  vector<Joystick> joysticks = {};
  Callbacks        callbacks = {};

  vec2i size                 = {0, 0};
  vec4i framebuffer_viewport = {0, 0, 0, 0};
  vec2i framebuffer_size     = {0, 0};
  int   gui_width            = 0;
  bool  gui_left             = true;

  // Shortcuts to run callbacks.
  void drop(const vector<string>& names) { callbacks.drop(*this, names); }
  void key(Key key, bool pressed) { callbacks.key(*this, key, pressed); }
  void click(bool left, bool pressed) { callbacks.click(*this, left, pressed); }
  void scroll(float amount) { callbacks.scroll(*this, amount); }
  void focus(int focused) { callbacks.focus(*this, focused); }
};

void init_window(Window& win, const vec2i& size, const string& title);
void delete_window(Window& win);

bool  should_window_close(const Window& win);
vec2f get_mouse_pos_normalized(const Window& win, bool isometric = false);
bool  is_key_pressed(const Window& win, Key key);

void update_window_size(Window& win);
void update_input(Input& input, const Window& win);
void update_input(Window& win);
void init_joysticks(vector<Joystick>& joysticks);
void init_joysticks(Window& win);
void update_joystick_input(vector<Joystick>& joysticks, const Window& win);
void update_joystick_input(Window& win);
void init_callbacks(Window& win);

void poll_events(const Window& win, bool wait);
void swap_buffers(const Window& win);
void run_draw_loop(
    Window& win, function<void(Window&)> draw, bool wait = false);

void update_camera(frame3f& frame, float& focus, const Window& win);

// Tools for building a user interface. Maybe put this in another file?
void init_gui(Window& win, int width, bool left = true);
void gui_begin(const Window& win, const char* name);
void gui_end(const Window& win);

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

}  // namespace window

#endif
