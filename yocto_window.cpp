#include "yocto_window.h"

#include "yocto_commonio.h"

#ifdef __APPLE__
#define GL_SILENCE_DEPRECATION
#endif
#include "ext/glad/glad.h"

// break

#include <GLFW/glfw3.h>

// break

#include "ext/imgui/imgui.h"
#include "ext/imgui/imgui_impl_glfw.h"
#include "ext/imgui/imgui_impl_opengl3.h"
#include "ext/imgui/imgui_internal.h"

#define CUTE_FILES_IMPLEMENTATION
#include "ext/cute_files.h"

namespace opengl {

void update_camera(frame3f& frame, float& focus, const Window& win) {
  auto last_pos    = win.input.mouse_last;
  auto mouse_pos   = win.input.mouse_pos;
  auto mouse_left  = win.input.mouse_left;
  auto mouse_right = win.input.mouse_right;
  auto alt_down    = win.input.modifier_alt;
  auto shift_down  = win.input.modifier_shift;

  // handle mouse and keyboard for navigation
  if ((mouse_left || mouse_right) && !alt_down) {
    auto dolly  = 0.0f;
    auto pan    = zero2f;
    auto rotate = zero2f;
    if (mouse_left && !shift_down) rotate = (mouse_pos - last_pos) / 100.0f;
    if (mouse_right) dolly = (mouse_pos.x - last_pos.x) / 100.0f;
    if (mouse_left && shift_down) pan = (mouse_pos - last_pos) * focus / 200.0f;
    pan.x    = -pan.x;
    rotate.y = -rotate.y;
    update_turntable(frame, focus, rotate, dolly, pan);
  }
}

// void _fw_drop_callback(GLFWwindow* glfw, int num, const char** paths) {
//  auto& win = *(const Window*)glfwGetWindowUserPointer(glfw);
//  if (win.drop_cb) {
//    auto pathv = vector<string>();
//    for (auto i = 0; i < num; i++) pathv.push_back(paths[i]);
//    win.drop_cb(win, pathv);
//  }
//}
//
// void _fw_key_callback(
//    GLFWwindow* glfw, int key, int scancode, int action, int mods) {
//  auto& win = *(const Window*)glfwGetWindowUserPointer(glfw);
//  if (win.key_cb) win.key_cb(win, (Key)key, (bool)action);
//}
//
// void _fw_click_callback(GLFWwindow* glfw, int button, int action, int mods)
// {
//  auto& win = *(const Window*)glfwGetWindowUserPointer(glfw);
//  if (win.click_cb)
//    win.click_cb(win, button == GLFW_MOUSE_BUTTON_LEFT, (bool)action);
//}
//
// void _fw_scroll_callback(GLFWwindow* glfw, double xoffset, double yoffset)
// {
//  auto& win = *(const Window*)glfwGetWindowUserPointer(glfw);
//  if (win.scroll_cb) win.scroll_cb(win, (float)yoffset);
//}

void init_window(Window& win, const vec2i& size, const string& title) {
  // init glfw
  if (!glfwInit())
    throw std::runtime_error("cannot initialize windowing system");
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#if __APPLE__
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

  // create window
  win     = Window{};
  win.win = glfwCreateWindow(size.x, size.y, title.c_str(), nullptr, nullptr);
  if (!win.win) throw std::runtime_error("cannot initialize windowing system");
  glfwMakeContextCurrent(win.win);
  glfwSwapInterval(1);  // Enable vsync

  // set user data
  glfwSetWindowUserPointer(win.win, &win);

  // set callbacks
  if (win.callbacks.refresh) {
    glfwSetWindowRefreshCallback(win.win, [](GLFWwindow* glfw) {
      auto win = (Window*)glfwGetWindowUserPointer(glfw);
      win->refresh();
    });
  }

  if (win.callbacks.drop) {
    glfwSetDropCallback(
        win.win, [](GLFWwindow* glfw, int num, const char** paths) {
          auto win   = (Window*)glfwGetWindowUserPointer(glfw);
          auto pathv = vector<string>();
          for (auto i = 0; i < num; i++) pathv.push_back(paths[i]);
          win->drop(pathv);
        });
  }

  if (win.callbacks.key) {
    glfwSetKeyCallback(win.win,
        [](GLFWwindow* glfw, int key, int scancode, int action, int mods) {
          auto win = (Window*)glfwGetWindowUserPointer(glfw);
          win->key(key, (bool)action);
        });
  }

  if (win.callbacks.click) {
    glfwSetMouseButtonCallback(
        win.win, [](GLFWwindow* glfw, int button, int action, int mods) {
          auto win = (Window*)glfwGetWindowUserPointer(glfw);
          win->click(button == GLFW_MOUSE_BUTTON_LEFT, (bool)action);
        });
  }

  if (win.callbacks.scroll) {
    glfwSetScrollCallback(
        win.win, [](GLFWwindow* glfw, double xoffset, double yoffset) {
          auto win = (Window*)glfwGetWindowUserPointer(glfw);
          win->scroll((float)yoffset);
        });
  }

  glfwSetWindowSizeCallback(
      win.win, [](GLFWwindow* glfw, int width, int height) {
        auto win = (Window*)glfwGetWindowUserPointer(glfw);
        glfwGetWindowSize(
            win->win, &win->input.window_size.x, &win->input.window_size.y);
        if (win->widgets_width) win->input.window_size.x -= win->widgets_width;
        glfwGetFramebufferSize(win->win, &win->input.framebuffer_viewport.z,
            &win->input.framebuffer_viewport.w);
        win->input.framebuffer_viewport.x = 0;
        win->input.framebuffer_viewport.y = 0;
        if (win->widgets_width) {
          auto win_size = zero2i;
          glfwGetWindowSize(win->win, &win_size.x, &win_size.y);
          auto offset = (int)(win->widgets_width *
                              (float)win->input.framebuffer_viewport.z /
                              win_size.x);
          win->input.framebuffer_viewport.z -= offset;
          if (win->widgets_left) win->input.framebuffer_viewport.x += offset;
        }
      });

  // init gl extensions
  if (!gladLoadGL())
    throw std::runtime_error("cannot initialize OpenGL extensions");

  // widgets
  bool widgets = 1;
  if (widgets) {
    ImGui::CreateContext();
    ImGui::GetIO().IniFilename       = nullptr;
    ImGui::GetStyle().WindowRounding = 0;
    ImGui_ImplGlfw_InitForOpenGL(win.win, true);
#ifndef __APPLE__
    ImGui_ImplOpenGL3_Init();
#else
    ImGui_ImplOpenGL3_Init("#version 330");
#endif
    ImGui::StyleColorsDark();
  }

  // call init callback
  if (win.callbacks.init) win.init();
}

void delete_window(Window& win) {
  glfwDestroyWindow(win.win);
  glfwTerminate();
  win.win = nullptr;
}

// void* get_user_pointer(const Window& win) { return win.user_ptr; }

// void set_drop_callback(Window& win, drop_callback drop_cb) {
//  win.drop_cb = drop_cb;
//  glfwSetDropCallback(win.win, _fw_drop_callback);
//}
//
// void set_key_callback(Window& win, key_callback cb) {
//  win.key_cb = cb;
//  glfwSetKeyCallback(win.win, _fw_key_callback);
//}
//
// void set_click_callback(Window& win, click_callback cb) {
//  win.click_cb = cb;
//  glfwSetMouseButtonCallback(win.win, _fw_click_callback);
//}
//
// void set_scroll_callback(Window& win, scroll_callback cb) {
//  win.scroll_cb = cb;
//  glfwSetScrollCallback(win.win, _fw_scroll_callback);
//}

vec2i get_framebuffer_size(const Window& win) {
  auto size = zero2i;
  glfwGetFramebufferSize(win.win, &size.x, &size.y);
  return size;
}

vec4i get_framebuffer_viewport(const Window& win) {
  auto viewport = zero4i;
  glfwGetFramebufferSize(win.win, &viewport.z, &viewport.w);
  return viewport;
}

vec2i get_window_size(const Window& win) {
  auto size = zero2i;
  glfwGetWindowSize(win.win, &size.x, &size.y);
  return size;
}

float get_framebuffer_aspect_ratio(const Window& win) {
  auto size = get_framebuffer_size(win);
  return (float)size.x / (float)size.y;
}

bool should_window_close(const Window& win) {
  return glfwWindowShouldClose(win.win);
}
void set_window_close(const Window& win, bool close) {
  glfwSetWindowShouldClose(win.win, close ? GLFW_TRUE : GLFW_FALSE);
}

bool draw_loop(Window& win, bool wait) {
  glfwSwapBuffers(win.win);
  process_events(win, wait);
  if (glfwWindowShouldClose(win.win)) return true;
  return false;
}

vec2f get_mouse_pos(const Window& win) {
  double mouse_posx, mouse_posy;
  glfwGetCursorPos(win.win, &mouse_posx, &mouse_posy);
  auto pos = vec2f{(float)mouse_posx, (float)mouse_posy};
  return pos;
}

vec2f get_mouse_pos_normalized(const Window& win) {
  double mouse_posx, mouse_posy;
  glfwGetCursorPos(win.win, &mouse_posx, &mouse_posy);
  auto  pos    = vec2f{(float)mouse_posx, (float)mouse_posy};
  auto  size   = get_window_size(win);
  auto  result = vec2f{2 * (pos.x / size.x) - 1, 1 - 2 * (pos.y / size.y)};
  float aspect = float(size.x) / size.y;
  result.x *= aspect;
  return result;
}

bool get_mouse_left(const Window& win) {
  return glfwGetMouseButton(win.win, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS;
}
bool get_mouse_right(const Window& win) {
  return glfwGetMouseButton(win.win, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS;
}

bool is_key_pressed(const Window& win, Key key) {
  return glfwGetKey(win.win, (int)key) == GLFW_PRESS;
}

bool get_alt_key(const Window& win) {
  return glfwGetKey(win.win, GLFW_KEY_LEFT_ALT) == GLFW_PRESS ||
         glfwGetKey(win.win, GLFW_KEY_RIGHT_ALT) == GLFW_PRESS;
}

bool get_shift_key(const Window& win) {
  return glfwGetKey(win.win, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS ||
         glfwGetKey(win.win, GLFW_KEY_RIGHT_SHIFT) == GLFW_PRESS;
}

void process_events(Window& win, bool wait) {
  // update input
  win.input.mouse_last = win.input.mouse_pos;
  auto mouse_posx = 0.0, mouse_posy = 0.0;
  glfwGetCursorPos(win.win, &mouse_posx, &mouse_posy);
  win.input.mouse_pos = vec2f{(float)mouse_posx, (float)mouse_posy};
  if (win.widgets_width && win.widgets_left)
    win.input.mouse_pos.x -= win.widgets_width;
  win.input.mouse_left = glfwGetMouseButton(win.win, GLFW_MOUSE_BUTTON_LEFT) ==
                         GLFW_PRESS;
  win.input.mouse_right = glfwGetMouseButton(
                              win.win, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS;
  win.input.modifier_alt =
      glfwGetKey(win.win, GLFW_KEY_LEFT_ALT) == GLFW_PRESS ||
      glfwGetKey(win.win, GLFW_KEY_RIGHT_ALT) == GLFW_PRESS;
  win.input.modifier_shift =
      glfwGetKey(win.win, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS ||
      glfwGetKey(win.win, GLFW_KEY_RIGHT_SHIFT) == GLFW_PRESS;
  win.input.modifier_ctrl =
      glfwGetKey(win.win, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS ||
      glfwGetKey(win.win, GLFW_KEY_RIGHT_CONTROL) == GLFW_PRESS;
  glfwGetWindowSize(
      win.win, &win.input.window_size.x, &win.input.window_size.y);
  if (win.widgets_width) win.input.window_size.x -= win.widgets_width;
  glfwGetFramebufferSize(win.win, &win.input.framebuffer_viewport.z,
      &win.input.framebuffer_viewport.w);
  win.input.framebuffer_viewport.x = 0;
  win.input.framebuffer_viewport.y = 0;
  if (win.widgets_width) {
    auto win_size = zero2i;
    glfwGetWindowSize(win.win, &win_size.x, &win_size.y);
    auto offset = (int)(win.widgets_width *
                        (float)win.input.framebuffer_viewport.z / win_size.x);
    win.input.framebuffer_viewport.z -= offset;
    if (win.widgets_left) win.input.framebuffer_viewport.x += offset;
  }
  if (win.widgets_width) {
    auto io                  = &ImGui::GetIO();
    win.input.widgets_active = io->WantTextInput || io->WantCaptureMouse ||
                               io->WantCaptureKeyboard;
  }

  // time
  win.input.clock_last = win.input.clock_now;
  win.input.clock_now =
      std::chrono::high_resolution_clock::now().time_since_epoch().count();
  win.input.time_now   = (double)win.input.clock_now / 1000000000.0;
  win.input.time_delta = (double)(win.input.clock_now - win.input.clock_last) /
                         1000000000.0;

  if (wait)
    glfwWaitEvents();
  else
    glfwPollEvents();
}

void swap_buffers(const Window& win) { glfwSwapBuffers(win.win); }

// }  // namespace yocto

// -----------------------------------------------------------------------------
// OPENGL WIDGETS
// -----------------------------------------------------------------------------
// namespace yocto {

void init_widgets(Window& win, int width, bool left) {
  // init widgets
  ImGui::CreateContext();
  ImGui::GetIO().IniFilename       = nullptr;
  ImGui::GetStyle().WindowRounding = 0;
  ImGui_ImplGlfw_InitForOpenGL(win.win, true);
#ifndef __APPLE__
  ImGui_ImplOpenGL3_Init();
#else
  ImGui_ImplOpenGL3_Init("#version 330");
#endif
  ImGui::StyleColorsDark();
  win.widgets_width = width;
  win.widgets_left  = left;
}

bool get_widgets_active(const Window& win) {
  auto io = &ImGui::GetIO();
  return io->WantTextInput || io->WantCaptureMouse || io->WantCaptureKeyboard;
}

void begin_widgets(
    const Window& win, const vec2f& position, const vec2f& size) {
  auto win_size = get_window_size(win);
  ImGui_ImplOpenGL3_NewFrame();
  ImGui_ImplGlfw_NewFrame();
  ImGui::NewFrame();
  if (win.widgets_left) {
    ImGui::SetNextWindowPos({position.x, position.y});
    ImGui::SetNextWindowSize({size.x, size.y});
    // ImGui::SetNextWindowSize({(float)win.widgets_width, (float)win_size.y});
  } else {
    ImGui::SetNextWindowPos({(float)(win_size.x - win.widgets_width), 0});
    ImGui::SetNextWindowSize({(float)win.widgets_width, (float)win_size.y});
  }
  // ImGui::SetNextWindowCollapsed(false);
  // ImGui::SetNextWindowBgAlpha(1);
  ImGui::Begin("widgets", nullptr,
      ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse |
          ImGuiWindowFlags_NoResize);
}

void begin_widgets(const Window& win) {
  auto win_size = get_window_size(win);
  begin_widgets(win, {0, 0}, {(float)win.widgets_width, (float)win_size.y});
}

void end_widgets(const Window& win) {
  ImGui::End();
  ImGui::Render();
  ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

bool begin_widgets_window(const Window& win, const char* title) {
  return ImGui::Begin(title, nullptr,
      // ImGuiWindowFlags_NoTitleBar |
      ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
          ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoSavedSettings);
}

bool begin_header(const Window& win, const char* lbl) {
  if (!ImGui::CollapsingHeader(lbl)) return false;
  ImGui::PushID(lbl);
  return true;
}
void end_header(const Window& win) { ImGui::PopID(); }

void open_modal(const Window& win, const char* lbl) { ImGui::OpenPopup(lbl); }
void clear_modal(const Window& win) { ImGui::CloseCurrentPopup(); }
bool begin_modal(const Window& win, const char* lbl) {
  return ImGui::BeginPopupModal(lbl);
}
void end_modal(const Window& win) { ImGui::EndPopup(); }
bool is_modal_open(const Window& win, const char* lbl) {
  return ImGui::IsPopupOpen(lbl);
}

bool draw_message(const Window& win, const char* lbl, const string& message) {
  if (ImGui::BeginPopupModal(lbl)) {
    auto open = true;
    ImGui::Text("%s", message.c_str());
    if (ImGui::Button("Ok")) {
      ImGui::CloseCurrentPopup();
      open = false;
    }
    ImGui::EndPopup();
    return open;
  } else {
    return false;
  }
}

std::deque<string> _message_queue = {};
std::mutex         _message_mutex;
void               push_message(const string& message) {
  std::lock_guard lock(_message_mutex);
  _message_queue.push_back(message);
}
void push_message(const Window& win, const string& message) {
  std::lock_guard lock(_message_mutex);
  _message_queue.push_back(message);
}
bool draw_messages(const Window& win) {
  std::lock_guard lock(_message_mutex);
  if (_message_queue.empty()) return false;
  if (!is_modal_open(win, "<message>")) {
    open_modal(win, "<message>");
    return true;
  } else if (ImGui::BeginPopupModal("<message>")) {
    ImGui::Text("%s", _message_queue.front().c_str());
    if (ImGui::Button("Ok")) {
      ImGui::CloseCurrentPopup();
      _message_queue.pop_front();
    }
    ImGui::EndPopup();
    return true;
  } else {
    return false;
  }
}

struct filedialog_state {
  string                     dirname       = "";
  string                     filename      = "";
  vector<pair<string, bool>> entries       = {};
  bool                       save          = false;
  bool                       remove_hidden = true;
  string                     filter        = "";
  vector<string>             extensions    = {};

  filedialog_state() {}
  filedialog_state(const string& dirname, const string& filename, bool save,
      const string& filter) {
    this->save = save;
    set_filter(filter);
    set_dirname(dirname);
    set_filename(filename);
  }
  void set_dirname(const string& name) {
    dirname = name;
    dirname = normalize_path(dirname);
    if (dirname == "") dirname = "./";
    if (dirname.back() != '/') dirname += '/';
    refresh();
  }
  void set_filename(const string& name) {
    filename = name;
    check_filename();
  }
  void set_filter(const string& flt) {
    auto globs = vector<string>{""};
    for (auto i = 0; i < flt.size(); i++) {
      if (flt[i] == ';') {
        globs.push_back("");
      } else {
        globs.back() += flt[i];
      }
    }
    filter = "";
    extensions.clear();
    for (auto pattern : globs) {
      if (pattern == "") continue;
      auto ext = get_extension(pattern);
      if (ext != "") {
        extensions.push_back(ext);
        filter += (filter == "") ? ("*." + ext) : (";*." + ext);
      }
    }
  }
  void check_filename() {
    if (filename.empty()) return;
    auto ext = get_extension(filename);
    if (std::find(extensions.begin(), extensions.end(), ext) ==
        extensions.end()) {
      filename = "";
      return;
    }
    if (!save && !exists_file(dirname + filename)) {
      filename = "";
      return;
    }
  }
  void select_entry(int idx) {
    if (entries[idx].second) {
      set_dirname(dirname + entries[idx].first);
    } else {
      set_filename(entries[idx].first);
    }
  }

  void refresh() {
    entries.clear();
    cf_dir_t dir;
    cf_dir_open(&dir, dirname.c_str());
    while (dir.has_next) {
      cf_file_t file;
      cf_read_file(&dir, &file);
      cf_dir_next(&dir);
      if (remove_hidden && file.name[0] == '.') continue;
      if (file.is_dir) {
        entries.push_back({file.name + "/"s, true});
      } else {
        entries.push_back({file.name, false});
      }
    }
    cf_dir_close(&dir);
    std::sort(entries.begin(), entries.end(), [](auto& a, auto& b) {
      if (a.second == b.second) return a.first < b.first;
      return a.second;
    });
  }

  string get_path() const { return dirname + filename; }
  bool   exists_file(const string& filename) {
    auto f = fopen(filename.c_str(), "r");
    if (!f) return false;
    fclose(f);
    return true;
  }
};
bool draw_filedialog(const Window& win, const char* lbl, string& path,
    bool save, const string& dirname, const string& filename,
    const string& filter) {
  static auto states = hash_map<string, filedialog_state>{};
  ImGui::SetNextWindowSize({500, 300}, ImGuiCond_FirstUseEver);
  if (ImGui::BeginPopupModal(lbl)) {
    if (states.find(lbl) == states.end()) {
      states[lbl] = filedialog_state{dirname, filename, save, filter};
    }
    auto& state = states.at(lbl);
    char  dir_buffer[1024];
    strcpy(dir_buffer, state.dirname.c_str());
    if (ImGui::InputText("dir", dir_buffer, sizeof(dir_buffer))) {
      state.set_dirname(dir_buffer);
    }
    auto current_item = -1;
    if (ImGui::ListBox(
            "entries", &current_item,
            [](void* data, int idx, const char** out_text) -> bool {
              auto& state = *(filedialog_state*)data;
              *out_text   = state.entries[idx].first.c_str();
              return true;
            },
            &state, (int)state.entries.size())) {
      state.select_entry(current_item);
    }
    char file_buffer[1024];
    strcpy(file_buffer, state.filename.c_str());
    if (ImGui::InputText("file", file_buffer, sizeof(file_buffer))) {
      state.set_filename(file_buffer);
    }
    char filter_buffer[1024];
    strcpy(filter_buffer, state.filter.c_str());
    if (ImGui::InputText("filter", filter_buffer, sizeof(filter_buffer))) {
      state.set_filter(filter_buffer);
    }
    auto ok = false, exit = false;
    if (ImGui::Button("Ok")) {
      path = state.dirname + state.filename;
      ok   = true;
      exit = true;
    }
    ImGui::SameLine();
    if (ImGui::Button("Cancel")) {
      exit = true;
    }
    if (exit) {
      ImGui::CloseCurrentPopup();
      states.erase(lbl);
    }
    ImGui::EndPopup();
    return ok;
  } else {
    return false;
  }
}
bool draw_filedialog_button(const Window& win, const char* button_lbl,
    bool button_active, const char* lbl, string& path, bool save,
    const string& dirname, const string& filename, const string& filter) {
  if (is_modal_open(win, lbl)) {
    return draw_filedialog(win, lbl, path, save, dirname, filename, filter);
  } else {
    if (draw_button(win, button_lbl, button_active)) {
      open_modal(win, lbl);
    }
    return false;
  }
}

bool draw_button(const Window& win, const char* lbl) {
  return ImGui::Button(lbl);
}
bool draw_button(const Window& win, const char* lbl, bool enabled) {
  if (enabled) {
    return ImGui::Button(lbl);
  } else {
    ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
    ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
    auto ok = ImGui::Button(lbl);
    ImGui::PopItemFlag();
    ImGui::PopStyleVar();
    return ok;
  }
}

void draw_label(const Window& win, const char* lbl, const string& text) {
  ImGui::LabelText(lbl, "%s", text.c_str());
}

void draw_separator(const Window& win) { ImGui::Separator(); }

void continue_line(const Window& win) { ImGui::SameLine(); }

bool draw_textinput(const Window& win, const char* lbl, string& value) {
  char buffer[4096];
  auto num = 0;
  for (auto c : value) buffer[num++] = c;
  buffer[num] = 0;
  auto edited = ImGui::InputText(lbl, buffer, sizeof(buffer));
  if (edited) value = buffer;
  return edited;
}

bool draw_slider(
    const Window& win, const char* lbl, float& value, float min, float max) {
  return ImGui::SliderFloat(lbl, &value, min, max);
}
bool draw_slider(
    const Window& win, const char* lbl, vec2f& value, float min, float max) {
  return ImGui::SliderFloat2(lbl, &value.x, min, max);
}
bool draw_slider(
    const Window& win, const char* lbl, vec3f& value, float min, float max) {
  return ImGui::SliderFloat3(lbl, &value.x, min, max);
}
bool draw_slider(
    const Window& win, const char* lbl, vec4f& value, float min, float max) {
  return ImGui::SliderFloat4(lbl, &value.x, min, max);
}

bool draw_slider(
    const Window& win, const char* lbl, int& value, int min, int max) {
  return ImGui::SliderInt(lbl, &value, min, max);
}
bool draw_slider(
    const Window& win, const char* lbl, vec2i& value, int min, int max) {
  return ImGui::SliderInt2(lbl, &value.x, min, max);
}
bool draw_slider(
    const Window& win, const char* lbl, vec3i& value, int min, int max) {
  return ImGui::SliderInt3(lbl, &value.x, min, max);
}
bool draw_slider(
    const Window& win, const char* lbl, vec4i& value, int min, int max) {
  return ImGui::SliderInt4(lbl, &value.x, min, max);
}

bool draw_dragger(const Window& win, const char* lbl, float& value, float speed,
    float min, float max) {
  return ImGui::DragFloat(lbl, &value, speed, min, max);
}
bool draw_dragger(const Window& win, const char* lbl, vec2f& value, float speed,
    float min, float max) {
  return ImGui::DragFloat2(lbl, &value.x, speed, min, max);
}
bool draw_dragger(const Window& win, const char* lbl, vec3f& value, float speed,
    float min, float max) {
  return ImGui::DragFloat3(lbl, &value.x, speed, min, max);
}
bool draw_dragger(const Window& win, const char* lbl, vec4f& value, float speed,
    float min, float max) {
  return ImGui::DragFloat4(lbl, &value.x, speed, min, max);
}

bool draw_dragger(const Window& win, const char* lbl, int& value, float speed,
    int min, int max) {
  return ImGui::DragInt(lbl, &value, speed, min, max);
}
bool draw_dragger(const Window& win, const char* lbl, vec2i& value, float speed,
    int min, int max) {
  return ImGui::DragInt2(lbl, &value.x, speed, min, max);
}
bool draw_dragger(const Window& win, const char* lbl, vec3i& value, float speed,
    int min, int max) {
  return ImGui::DragInt3(lbl, &value.x, speed, min, max);
}
bool draw_dragger(const Window& win, const char* lbl, vec4i& value, float speed,
    int min, int max) {
  return ImGui::DragInt4(lbl, &value.x, speed, min, max);
}

bool draw_checkbox(const Window& win, const char* lbl, bool& value) {
  return ImGui::Checkbox(lbl, &value);
}

bool draw_coloredit(const Window& win, const char* lbl, vec3f& value) {
  auto flags = ImGuiColorEditFlags_Float;
  return ImGui::ColorEdit3(lbl, &value.x, flags);
}

bool draw_coloredit(const Window& win, const char* lbl, vec4f& value) {
  auto flags = ImGuiColorEditFlags_Float;
  return ImGui::ColorEdit4(lbl, &value.x, flags);
}

bool draw_hdrcoloredit(const Window& win, const char* lbl, vec3f& value) {
  auto color    = value;
  auto exposure = 0.0f;
  auto scale    = max(color);
  if (scale > 1) {
    color /= scale;
    exposure = yocto::log2(scale);
  }
  auto edit_exposure = draw_slider(
      win, (lbl + " [exp]"s).c_str(), exposure, 0, 10);
  auto edit_color = draw_coloredit(win, (lbl + " [col]"s).c_str(), color);
  if (edit_exposure || edit_color) {
    value = color * yocto::exp2(exposure);
    return true;
  } else {
    return false;
  }
}
bool draw_hdrcoloredit(const Window& win, const char* lbl, vec4f& value) {
  auto color    = value;
  auto exposure = 0.0f;
  auto scale    = max(xyz(color));
  if (scale > 1) {
    xyz(color) /= scale;
    exposure = yocto::log2(scale);
  }
  auto edit_exposure = draw_slider(
      win, (lbl + " [exp]"s).c_str(), exposure, 0, 10);
  auto edit_color = draw_coloredit(win, (lbl + " [col]"s).c_str(), color);
  if (edit_exposure || edit_color) {
    xyz(value) = xyz(color) * yocto::exp2(exposure);
    value.w    = color.w;
    return true;
  } else {
    return false;
  }
}

bool draw_combobox(const Window& win, const char* lbl, int& value,
    const vector<string>& labels) {
  if (!ImGui::BeginCombo(lbl, labels[value].c_str())) return false;
  auto old_val = value;
  for (auto i = 0; i < labels.size(); i++) {
    ImGui::PushID(i);
    if (ImGui::Selectable(labels[i].c_str(), value == i)) value = i;
    if (value == i) ImGui::SetItemDefaultFocus();
    ImGui::PopID();
  }
  ImGui::EndCombo();
  return value != old_val;
}

bool draw_combobox(const Window& win, const char* lbl, string& value,
    const vector<string>& labels) {
  if (!ImGui::BeginCombo(lbl, value.c_str())) return false;
  auto old_val = value;
  for (auto i = 0; i < labels.size(); i++) {
    ImGui::PushID(i);
    if (ImGui::Selectable(labels[i].c_str(), value == labels[i]))
      value = labels[i];
    if (value == labels[i]) ImGui::SetItemDefaultFocus();
    ImGui::PopID();
  }
  ImGui::EndCombo();
  return value != old_val;
}

bool draw_combobox(const Window& win, const char* lbl, int& idx, int num,
    const std::function<const char*(int)>& labels, bool include_null) {
  if (num <= 0) idx = -1;
  if (!ImGui::BeginCombo(lbl, idx >= 0 ? labels(idx) : "<none>")) return false;
  auto old_idx = idx;
  if (include_null) {
    ImGui::PushID(100000);
    if (ImGui::Selectable("<none>", idx < 0)) idx = -1;
    if (idx < 0) ImGui::SetItemDefaultFocus();
    ImGui::PopID();
  }
  for (auto i = 0; i < num; i++) {
    ImGui::PushID(i);
    if (ImGui::Selectable(labels(i), idx == i)) idx = i;
    if (idx == i) ImGui::SetItemDefaultFocus();
    ImGui::PopID();
  }
  ImGui::EndCombo();
  return idx != old_idx;
}

void draw_histogram(
    const Window& win, const char* lbl, const float* values, int count) {
  ImGui::PlotHistogram(lbl, values, count);
}
void draw_histogram(
    const Window& win, const char* lbl, const vector<float>& values) {
  ImGui::PlotHistogram(lbl, values.data(), (int)values.size(), 0, nullptr,
      flt_max, flt_max, {0, 0}, 4);
}
void draw_histogram(
    const Window& win, const char* lbl, const vector<vec2f>& values) {
  ImGui::PlotHistogram((lbl + " x"s).c_str(), (const float*)values.data() + 0,
      (int)values.size(), 0, nullptr, flt_max, flt_max, {0, 0}, sizeof(vec2f));
  ImGui::PlotHistogram((lbl + " y"s).c_str(), (const float*)values.data() + 1,
      (int)values.size(), 0, nullptr, flt_max, flt_max, {0, 0}, sizeof(vec2f));
}
void draw_histogram(
    const Window& win, const char* lbl, const vector<vec3f>& values) {
  ImGui::PlotHistogram((lbl + " x"s).c_str(), (const float*)values.data() + 0,
      (int)values.size(), 0, nullptr, flt_max, flt_max, {0, 0}, sizeof(vec3f));
  ImGui::PlotHistogram((lbl + " y"s).c_str(), (const float*)values.data() + 1,
      (int)values.size(), 0, nullptr, flt_max, flt_max, {0, 0}, sizeof(vec3f));
  ImGui::PlotHistogram((lbl + " z"s).c_str(), (const float*)values.data() + 2,
      (int)values.size(), 0, nullptr, flt_max, flt_max, {0, 0}, sizeof(vec3f));
}
void draw_histogram(
    const Window& win, const char* lbl, const vector<vec4f>& values) {
  ImGui::PlotHistogram((lbl + " x"s).c_str(), (const float*)values.data() + 0,
      (int)values.size(), 0, nullptr, flt_max, flt_max, {0, 0}, sizeof(vec4f));
  ImGui::PlotHistogram((lbl + " y"s).c_str(), (const float*)values.data() + 1,
      (int)values.size(), 0, nullptr, flt_max, flt_max, {0, 0}, sizeof(vec4f));
  ImGui::PlotHistogram((lbl + " z"s).c_str(), (const float*)values.data() + 2,
      (int)values.size(), 0, nullptr, flt_max, flt_max, {0, 0}, sizeof(vec4f));
  ImGui::PlotHistogram((lbl + " w"s).c_str(), (const float*)values.data() + 3,
      (int)values.size(), 0, nullptr, flt_max, flt_max, {0, 0}, sizeof(vec4f));
}

// https://github.com/ocornut/imgui/issues/300
struct ImGuiAppLog {
  ImGuiTextBuffer Buf;
  ImGuiTextFilter Filter;
  ImVector<int>   LineOffsets;  // Index to lines offset
  bool            ScrollToBottom;

  void Clear() {
    Buf.clear();
    LineOffsets.clear();
  }

  void AddLog(const char* msg, const char* lbl) {
    int old_size = Buf.size();
    Buf.appendf("[%s] %s\n", lbl, msg);
    for (int new_size = Buf.size(); old_size < new_size; old_size++)
      if (Buf[old_size] == '\n') LineOffsets.push_back(old_size);
    ScrollToBottom = true;
  }

  void Draw() {
    if (ImGui::Button("Clear")) Clear();
    ImGui::SameLine();
    bool copy = ImGui::Button("Copy");
    ImGui::SameLine();
    Filter.Draw("Filter", -100.0f);
    ImGui::Separator();
    ImGui::BeginChild("scrolling");
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 1));
    if (copy) ImGui::LogToClipboard();

    if (Filter.IsActive()) {
      const char* buf_begin = Buf.begin();
      const char* line      = buf_begin;
      for (int line_no = 0; line != NULL; line_no++) {
        const char* line_end = (line_no < LineOffsets.Size)
                                   ? buf_begin + LineOffsets[line_no]
                                   : NULL;
        if (Filter.PassFilter(line, line_end))
          ImGui::TextUnformatted(line, line_end);
        line = line_end && line_end[1] ? line_end + 1 : NULL;
      }
    } else {
      ImGui::TextUnformatted(Buf.begin());
    }

    if (ScrollToBottom) ImGui::SetScrollHere(1.0f);
    ScrollToBottom = false;
    ImGui::PopStyleVar();
    ImGui::EndChild();
  }
  void Draw(const char* title, bool* p_opened = NULL) {
    ImGui::SetNextWindowSize(ImVec2(500, 400), ImGuiSetCond_FirstUseEver);
    ImGui::Begin(title, p_opened);
    Draw();
    ImGui::End();
  }
};

std::mutex  _log_mutex;
ImGuiAppLog _log_widget;
void        log_info(const Window& win, const string& msg) {
  _log_mutex.lock();
  _log_widget.AddLog(msg.c_str(), "info");
  _log_mutex.unlock();
}
void log_error(const Window& win, const string& msg) {
  _log_mutex.lock();
  _log_widget.AddLog(msg.c_str(), "errn");
  _log_mutex.unlock();
}
void clear_logs(const Window& win) {
  _log_mutex.lock();
  _log_widget.Clear();
  _log_mutex.unlock();
}
void draw_log(const Window& win) {
  _log_mutex.lock();
  _log_widget.Draw();
  _log_mutex.unlock();
}

// void update_input() {
//   // update input
//   win->input.mouse_last = win->input.mouse_pos;
//   auto mouse_posx = 0.0, mouse_posy = 0.0;
//   glfwGetCursorPos(win.win, &mouse_posx, &mouse_posy);
//   win->input.mouse_pos = vec2f{(float)mouse_posx, (float)mouse_posy};
//   if (win->widgets_width && win->widgets_left)
//     win->input.mouse_pos.x -= win->widgets_width;
//   win->input.mouse_left = glfwGetMouseButton(
//                               win.win, GLFW_MOUSE_BUTTON_LEFT) ==
//                               GLFW_PRESS;
//   win->input.mouse_right = glfwGetMouseButton(
//                                win.win, GLFW_MOUSE_BUTTON_RIGHT) ==
//                                GLFW_PRESS;
//   win->input.modifier_alt =
//       glfwGetKey(win.win, GLFW_KEY_LEFT_ALT) == GLFW_PRESS ||
//       glfwGetKey(win.win, GLFW_KEY_RIGHT_ALT) == GLFW_PRESS;
//   win->input.modifier_shift =
//       glfwGetKey(win.win, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS ||
//       glfwGetKey(win.win, GLFW_KEY_RIGHT_SHIFT) == GLFW_PRESS;
//   win->input.modifier_ctrl =
//       glfwGetKey(win.win, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS ||
//       glfwGetKey(win.win, GLFW_KEY_RIGHT_CONTROL) == GLFW_PRESS;
//   glfwGetWindowSize(
//       win.win, &win->input.window_size.x, &win->input.window_size.y);
//   if (win->widgets_width) win->input.window_size.x -= win->widgets_width;
//   glfwGetFramebufferSize(win.win, &win->input.framebuffer_viewport.z,
//       &win->input.framebuffer_viewport.w);
//   win->input.framebuffer_viewport.x = 0;
//   win->input.framebuffer_viewport.y = 0;
//   if (win->widgets_width) {
//     auto win_size = zero2i;
//     glfwGetWindowSize(win.win, &win_size.x, &win_size.y);
//     auto offset = (int)(win->widgets_width *
//                         (float)win->input.framebuffer_viewport.z /
//                         win_size.x);
//     win->input.framebuffer_viewport.z -= offset;
//     if (win->widgets_left) win->input.framebuffer_viewport.x += offset;
//   }
//   if (win->widgets_width) {
//     auto io                   = &ImGui::GetIO();
//     win->input.widgets_active = io->WantTextInput || io->WantCaptureMouse ||
//                                 io->WantCaptureKeyboard;
//   }

//   // time
//   win->input.clock_last = win->input.clock_now;
//   win->input.clock_now =
//       std::chrono::high_resolution_clock::now().time_since_epoch().count();
//   win->input.time_now = (double)win->input.clock_now / 1000000000.0;
//   win->input.time_delta =
//       (double)(win->input.clock_now - win->input.clock_last) / 1000000000.0;
// }

}  // namespace opengl
