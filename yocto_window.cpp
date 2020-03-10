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
  win      = Window{};
  win.glfw = glfwCreateWindow(size.x, size.y, title.c_str(), nullptr, nullptr);
  if (!win.glfw) throw std::runtime_error("cannot initialize windowing system");
  glfwMakeContextCurrent(win.glfw);
  glfwSwapInterval(1);  // Enable vsync

  // set user data
  glfwSetWindowUserPointer(win.glfw, &win);

  // set callbacks
  if (win.callbacks.refresh) {
    glfwSetWindowRefreshCallback(win.glfw, [](GLFWwindow* glfw) {
      auto win = (Window*)glfwGetWindowUserPointer(glfw);
      win->refresh();
    });
  }

  if (win.callbacks.drop) {
    glfwSetDropCallback(
        win.glfw, [](GLFWwindow* glfw, int num, const char** paths) {
          auto win   = (Window*)glfwGetWindowUserPointer(glfw);
          auto pathv = vector<string>();
          for (auto i = 0; i < num; i++) pathv.push_back(paths[i]);
          win->drop(pathv);
        });
  }

  if (win.callbacks.key) {
    glfwSetKeyCallback(win.glfw,
        [](GLFWwindow* glfw, int key, int scancode, int action, int mods) {
          auto win = (Window*)glfwGetWindowUserPointer(glfw);
          win->key(key, (bool)action);
        });
  }

  if (win.callbacks.click) {
    glfwSetMouseButtonCallback(
        win.glfw, [](GLFWwindow* glfw, int button, int action, int mods) {
          auto win = (Window*)glfwGetWindowUserPointer(glfw);
          win->click(button == GLFW_MOUSE_BUTTON_LEFT, (bool)action);
        });
  }

  if (win.callbacks.scroll) {
    glfwSetScrollCallback(
        win.glfw, [](GLFWwindow* glfw, double xoffset, double yoffset) {
          auto win = (Window*)glfwGetWindowUserPointer(glfw);
          win->scroll((float)yoffset);
        });
  }

  glfwSetWindowSizeCallback(
      win.glfw, [](GLFWwindow* glfw, int width, int height) {
        auto win = (Window*)glfwGetWindowUserPointer(glfw);
        glfwGetWindowSize(win->glfw, &win->window_size.x, &win->window_size.y);
        if (win->widgets_width) win->window_size.x -= win->widgets_width;
        glfwGetFramebufferSize(win->glfw, &win->framebuffer_viewport.z,
            &win->framebuffer_viewport.w);
        win->framebuffer_viewport.x = 0;
        win->framebuffer_viewport.y = 0;
        if (win->widgets_width) {
          auto win_size = zero2i;
          glfwGetWindowSize(win->glfw, &win_size.x, &win_size.y);
          auto offset = (int)(win->widgets_width *
                              (float)win->framebuffer_viewport.z / win_size.x);
          win->framebuffer_viewport.z -= offset;
          if (win->widgets_left) win->framebuffer_viewport.x += offset;
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
    ImGui_ImplGlfw_InitForOpenGL(win.glfw, true);
#ifndef __APPLE__
    ImGui_ImplOpenGL3_Init();
#else
    ImGui_ImplOpenGL3_Init("#version 330");
#endif
    ImGui::StyleColorsDark();
  }
  update_input(win);

  // call init callback
  if (win.callbacks.init) win.init();
}

void delete_window(Window& win) {
  glfwDestroyWindow(win.glfw);
  glfwTerminate();
  win.glfw = nullptr;
}

bool should_window_close(const Window& win) {
  return glfwWindowShouldClose(win.glfw);
}

void poll_events(const Window& win, bool wait) {
  if (wait)
    glfwWaitEvents();
  else
    glfwPollEvents();
}

void run_draw_loop(Window& win, bool wait) {
  while (!should_window_close(win)) {
    update_input(win);
    win.draw();
    win.gui();
    swap_buffers(win);
    poll_events(win, wait);
  }
}

vec2f get_mouse_pos_normalized(const Window& win) {
  auto& pos    = win.input.mouse_pos;
  auto  size   = win.window_size;
  auto  result = vec2f{2 * (pos.x / size.x) - 1, 1 - 2 * (pos.y / size.y)};
  result.x *= win.window_aspect;
  return result;
}

bool is_key_pressed(const Window& win, Key key) {
  return glfwGetKey(win.glfw, (int)key) == GLFW_PRESS;
}

void update_window_size(Window& win) {
  auto& glfw = win.glfw;
  glfwGetWindowSize(glfw, &win.window_size.x, &win.window_size.y);
  if (win.widgets_width) win.window_size.x -= win.widgets_width;
  glfwGetFramebufferSize(
      glfw, &win.framebuffer_viewport.z, &win.framebuffer_viewport.w);
  win.framebuffer_viewport.x = 0;
  win.framebuffer_viewport.y = 0;
  if (win.widgets_width) {
    auto win_size = zero2i;
    glfwGetWindowSize(glfw, &win_size.x, &win_size.y);
    auto offset = (int)(win.widgets_width * (float)win.framebuffer_viewport.z /
                        win_size.x);
    win.framebuffer_viewport.z -= offset;
    if (win.widgets_left) win.framebuffer_viewport.x += offset;
  }
  win.window_aspect    = float(win.window_size.x) / float(win.window_size.y);
  win.framebuffer_size = {
      win.framebuffer_viewport.z - win.framebuffer_viewport.x,
      win.framebuffer_viewport.w - win.framebuffer_viewport.y};
  win.framebuffer_aspect = float(win.framebuffer_size.x) /
                           float(win.framebuffer_size.y);
}

void update_input(Window& win) {
  // update input
  win.input.mouse_last = win.input.mouse_pos;
  auto  mouse_posx = 0.0, mouse_posy = 0.0;
  auto& glfw = win.glfw;
  glfwGetCursorPos(glfw, &mouse_posx, &mouse_posy);
  win.input.mouse_pos = vec2f{(float)mouse_posx, (float)mouse_posy};
  if (win.widgets_width && win.widgets_left)
    win.input.mouse_pos.x -= win.widgets_width;
  win.input.mouse_left = glfwGetMouseButton(glfw, GLFW_MOUSE_BUTTON_LEFT) ==
                         GLFW_PRESS;
  win.input.mouse_right = glfwGetMouseButton(glfw, GLFW_MOUSE_BUTTON_RIGHT) ==
                          GLFW_PRESS;
  win.input.modifier_alt = glfwGetKey(glfw, GLFW_KEY_LEFT_ALT) == GLFW_PRESS ||
                           glfwGetKey(glfw, GLFW_KEY_RIGHT_ALT) == GLFW_PRESS;
  win.input.modifier_shift =
      glfwGetKey(glfw, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS ||
      glfwGetKey(glfw, GLFW_KEY_RIGHT_SHIFT) == GLFW_PRESS;
  win.input.modifier_ctrl =
      glfwGetKey(glfw, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS ||
      glfwGetKey(glfw, GLFW_KEY_RIGHT_CONTROL) == GLFW_PRESS;

  if (win.widgets_width) {
    auto io                 = &ImGui::GetIO();
    win.input.is_gui_active = io->WantTextInput || io->WantCaptureMouse ||
                              io->WantCaptureKeyboard;
  }

  update_window_size(win);

  // time
  win.input.clock_last = win.input.clock_now;
  win.input.clock_now =
      std::chrono::high_resolution_clock::now().time_since_epoch().count();
  win.input.time_now   = (double)win.input.clock_now / 1e9;
  win.input.time_delta = (double)(win.input.clock_now - win.input.clock_last) /
                         1e9;
}

void swap_buffers(const Window& win) { glfwSwapBuffers(win.glfw); }

// -----------------------------------------------------------------------------
// OPENGL WIDGETS
// -----------------------------------------------------------------------------

void init_glwidgets(Window& win, int width, bool left) {
  // init widgets
  ImGui::CreateContext();
  ImGui::GetIO().IniFilename       = nullptr;
  ImGui::GetStyle().WindowRounding = 0;
  ImGui_ImplGlfw_InitForOpenGL(win.glfw, true);
#ifndef __APPLE__
  ImGui_ImplOpenGL3_Init();
#else
  ImGui_ImplOpenGL3_Init("#version 330");
#endif
  ImGui::StyleColorsDark();
  win.widgets_width = width;
  win.widgets_left  = left;
}

bool begin_header(Window& win, const char* lbl) {
  if (!ImGui::CollapsingHeader(lbl)) return false;
  ImGui::PushID(lbl);
  return true;
}
void end_header(Window& win) { ImGui::PopID(); }

void open_glmodal(Window& win, const char* lbl) { ImGui::OpenPopup(lbl); }
void clear_glmodal(Window& win) { ImGui::CloseCurrentPopup(); }
bool begin_glmodal(Window& win, const char* lbl) {
  return ImGui::BeginPopupModal(lbl);
}
void end_glmodal(Window& win) { ImGui::EndPopup(); }
bool is_glmodal_open(Window& win, const char* lbl) {
  return ImGui::IsPopupOpen(lbl);
}

bool gui_message(Window& win, const char* lbl, const std::string& message) {
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

std::deque<std::string> _message_queue = {};
std::mutex              _message_mutex;
void                    push_message(Window& win, const std::string& message) {
  std::lock_guard lock(_message_mutex);
  _message_queue.push_back(message);
}
bool gui_messages(Window& win) {
  std::lock_guard lock(_message_mutex);
  if (_message_queue.empty()) return false;
  if (!is_glmodal_open(win, "<message>")) {
    open_glmodal(win, "<message>");
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

// Utility to normalize a path
static inline std::string normalize_path(const std::string& filename_) {
  auto filename = filename_;
  for (auto& c : filename)

    if (c == '\\') c = '/';
  if (filename.size() > 1 && filename[0] == '/' && filename[1] == '/') {
    throw std::invalid_argument("absolute paths are not supported");
    return filename_;
  }
  if (filename.size() > 3 && filename[1] == ':' && filename[2] == '/' &&
      filename[3] == '/') {
    throw std::invalid_argument("absolute paths are not supported");
    return filename_;
  }
  auto pos = (size_t)0;
  while ((pos = filename.find("//")) != filename.npos)
    filename = filename.substr(0, pos) + filename.substr(pos + 1);
  return filename;
}

// Get extension (not including '.').
static std::string get_extension(const std::string& filename_) {
  auto filename = normalize_path(filename_);
  auto pos      = filename.rfind('.');
  if (pos == std::string::npos) return "";
  return filename.substr(pos);
}

struct filedialog_state {
  std::string                               dirname       = "";
  std::string                               filename      = "";
  std::vector<std::pair<std::string, bool>> entries       = {};
  bool                                      save          = false;
  bool                                      remove_hidden = true;
  std::string                               filter        = "";
  std::vector<std::string>                  extensions    = {};

  filedialog_state() {}
  filedialog_state(const std::string& dirname, const std::string& filename,
      bool save, const std::string& filter) {
    this->save = save;
    set_filter(filter);
    set_dirname(dirname);
    set_filename(filename);
  }
  void set_dirname(const std::string& name) {
    dirname = name;
    dirname = normalize_path(dirname);
    if (dirname == "") dirname = "./";
    if (dirname.back() != '/') dirname += '/';
    refresh();
  }
  void set_filename(const std::string& name) {
    filename = name;
    check_filename();
  }
  void set_filter(const std::string& flt) {
    auto globs = std::vector<std::string>{""};
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

  std::string get_path() const { return dirname + filename; }
  bool        exists_file(const std::string& filename) {
    auto f = fopen(filename.c_str(), "r");
    if (!f) return false;
    fclose(f);
    return true;
  }
};
bool gui_filedialog(Window& win, const char* lbl, std::string& path, bool save,
    const std::string& dirname, const std::string& filename,
    const std::string& filter) {
  static auto states = std::unordered_map<std::string, filedialog_state>{};
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
bool gui_filedialog_button(Window& win, const char* button_lbl,
    bool button_active, const char* lbl, std::string& path, bool save,
    const std::string& dirname, const std::string& filename,
    const std::string& filter) {
  if (is_glmodal_open(win, lbl)) {
    return gui_filedialog(win, lbl, path, save, dirname, filename, filter);
  } else {
    if (gui_button(win, button_lbl, button_active)) {
      open_glmodal(win, lbl);
    }
    return false;
  }
}

bool gui_button(Window& win, const char* lbl, bool enabled) {
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

void gui_label(Window& win, const char* lbl, const std::string& label) {
  ImGui::LabelText(lbl, "%s", label.c_str());
}

void gui_separator(Window& win) { ImGui::Separator(); }

void continue_line(Window& win) { ImGui::SameLine(); }

bool gui_textinput(Window& win, const char* lbl, std::string& value) {
  char buffer[4096];
  auto num = 0;
  for (auto c : value) buffer[num++] = c;
  buffer[num] = 0;
  auto edited = ImGui::InputText(lbl, buffer, sizeof(buffer));
  if (edited) value = buffer;
  return edited;
}

bool gui_slider(
    Window& win, const char* lbl, float& value, float min, float max) {
  return ImGui::SliderFloat(lbl, &value, min, max);
}
bool gui_slider(
    Window& win, const char* lbl, vec2f& value, float min, float max) {
  return ImGui::SliderFloat2(lbl, &value.x, min, max);
}
bool gui_slider(
    Window& win, const char* lbl, vec3f& value, float min, float max) {
  return ImGui::SliderFloat3(lbl, &value.x, min, max);
}
bool gui_slider(
    Window& win, const char* lbl, vec4f& value, float min, float max) {
  return ImGui::SliderFloat4(lbl, &value.x, min, max);
}

bool gui_slider(Window& win, const char* lbl, int& value, int min, int max) {
  return ImGui::SliderInt(lbl, &value, min, max);
}
bool gui_slider(Window& win, const char* lbl, vec2i& value, int min, int max) {
  return ImGui::SliderInt2(lbl, &value.x, min, max);
}
bool gui_slider(Window& win, const char* lbl, vec3i& value, int min, int max) {
  return ImGui::SliderInt3(lbl, &value.x, min, max);
}
bool gui_slider(Window& win, const char* lbl, vec4i& value, int min, int max) {
  return ImGui::SliderInt4(lbl, &value.x, min, max);
}

bool gui_dragger(Window& win, const char* lbl, float& value, float speed,
    float min, float max) {
  return ImGui::DragFloat(lbl, &value, speed, min, max);
}
bool gui_dragger(Window& win, const char* lbl, vec2f& value, float speed,
    float min, float max) {
  return ImGui::DragFloat2(lbl, &value.x, speed, min, max);
}
bool gui_dragger(Window& win, const char* lbl, vec3f& value, float speed,
    float min, float max) {
  return ImGui::DragFloat3(lbl, &value.x, speed, min, max);
}
bool gui_dragger(Window& win, const char* lbl, vec4f& value, float speed,
    float min, float max) {
  return ImGui::DragFloat4(lbl, &value.x, speed, min, max);
}

bool gui_dragger(
    Window& win, const char* lbl, int& value, float speed, int min, int max) {
  return ImGui::DragInt(lbl, &value, speed, min, max);
}
bool gui_dragger(
    Window& win, const char* lbl, vec2i& value, float speed, int min, int max) {
  return ImGui::DragInt2(lbl, &value.x, speed, min, max);
}
bool gui_dragger(
    Window& win, const char* lbl, vec3i& value, float speed, int min, int max) {
  return ImGui::DragInt3(lbl, &value.x, speed, min, max);
}
bool gui_dragger(
    Window& win, const char* lbl, vec4i& value, float speed, int min, int max) {
  return ImGui::DragInt4(lbl, &value.x, speed, min, max);
}

bool gui_checkbox(Window& win, const char* lbl, bool& value) {
  return ImGui::Checkbox(lbl, &value);
}

bool gui_coloredit(Window& win, const char* lbl, vec3f& value) {
  auto flags = ImGuiColorEditFlags_Float;
  return ImGui::ColorEdit3(lbl, &value.x, flags);
}

bool gui_coloredit(Window& win, const char* lbl, vec4f& value) {
  auto flags = ImGuiColorEditFlags_Float;
  return ImGui::ColorEdit4(lbl, &value.x, flags);
}

bool gui_hdrcoloredit(Window& win, const char* lbl, vec3f& value) {
  auto color    = value;
  auto exposure = 0.0f;
  auto scale    = max(color);
  if (scale > 1) {
    color /= scale;
    exposure = yocto::log2(scale);
  }
  auto edit_exposure = gui_slider(
      win, (lbl + " [exp]"s).c_str(), exposure, 0, 10);
  auto edit_color = gui_coloredit(win, (lbl + " [col]"s).c_str(), color);
  if (edit_exposure || edit_color) {
    value = color * yocto::exp2(exposure);
    return true;
  } else {
    return false;
  }
}
bool gui_hdrcoloredit(Window& win, const char* lbl, vec4f& value) {
  auto color    = value;
  auto exposure = 0.0f;
  auto scale    = max(xyz(color));
  if (scale > 1) {
    xyz(color) /= scale;
    exposure = yocto::log2(scale);
  }
  auto edit_exposure = gui_slider(
      win, (lbl + " [exp]"s).c_str(), exposure, 0, 10);
  auto edit_color = gui_coloredit(win, (lbl + " [col]"s).c_str(), color);
  if (edit_exposure || edit_color) {
    xyz(value) = xyz(color) * yocto::exp2(exposure);
    value.w    = color.w;
    return true;
  } else {
    return false;
  }
}

bool gui_combobox(Window& win, const char* lbl, int& value,
    const std::vector<std::string>& labels) {
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

bool gui_combobox(Window& win, const char* lbl, std::string& value,
    const std::vector<std::string>& labels) {
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

bool gui_combobox(Window& win, const char* lbl, int& idx, int num,
    const std::function<std::string(int)>& labels, bool include_null) {
  if (num <= 0) idx = -1;
  if (!ImGui::BeginCombo(lbl, idx >= 0 ? labels(idx).c_str() : "<none>"))
    return false;
  auto old_idx = idx;
  if (include_null) {
    ImGui::PushID(100000);
    if (ImGui::Selectable("<none>", idx < 0)) idx = -1;
    if (idx < 0) ImGui::SetItemDefaultFocus();
    ImGui::PopID();
  }
  for (auto i = 0; i < num; i++) {
    ImGui::PushID(i);
    if (ImGui::Selectable(labels(i).c_str(), idx == i)) idx = i;
    if (idx == i) ImGui::SetItemDefaultFocus();
    ImGui::PopID();
  }
  ImGui::EndCombo();
  return idx != old_idx;
}

void gui_progressbar(Window& win, const char* lbl, float fraction) {
  ImGui::PushStyleColor(ImGuiCol_PlotHistogram, ImVec4(0.5, 0.5, 1, 0.25));
  ImGui::ProgressBar(fraction, ImVec2(0.0f, 0.0f));
  ImGui::SameLine(0.0f, ImGui::GetStyle().ItemInnerSpacing.x);
  ImGui::Text(lbl, ImVec2(0.0f, 0.0f));
  ImGui::PopStyleColor(1);
}

void gui_histogram(
    Window& win, const char* lbl, const float* values, int count) {
  ImGui::PlotHistogram(lbl, values, count);
}
void gui_histogram(
    Window& win, const char* lbl, const std::vector<float>& values) {
  ImGui::PlotHistogram(lbl, values.data(), (int)values.size(), 0, nullptr,
      flt_max, flt_max, {0, 0}, 4);
}
void gui_histogram(
    Window& win, const char* lbl, const std::vector<vec2f>& values) {
  ImGui::PlotHistogram((lbl + " x"s).c_str(), (const float*)values.data() + 0,
      (int)values.size(), 0, nullptr, flt_max, flt_max, {0, 0}, sizeof(vec2f));
  ImGui::PlotHistogram((lbl + " y"s).c_str(), (const float*)values.data() + 1,
      (int)values.size(), 0, nullptr, flt_max, flt_max, {0, 0}, sizeof(vec2f));
}
void gui_histogram(
    Window& win, const char* lbl, const std::vector<vec3f>& values) {
  ImGui::PlotHistogram((lbl + " x"s).c_str(), (const float*)values.data() + 0,
      (int)values.size(), 0, nullptr, flt_max, flt_max, {0, 0}, sizeof(vec3f));
  ImGui::PlotHistogram((lbl + " y"s).c_str(), (const float*)values.data() + 1,
      (int)values.size(), 0, nullptr, flt_max, flt_max, {0, 0}, sizeof(vec3f));
  ImGui::PlotHistogram((lbl + " z"s).c_str(), (const float*)values.data() + 2,
      (int)values.size(), 0, nullptr, flt_max, flt_max, {0, 0}, sizeof(vec3f));
}
void gui_histogram(
    Window& win, const char* lbl, const std::vector<vec4f>& values) {
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
void        log_info(Window& win, const std::string& msg) {
  _log_mutex.lock();
  _log_widget.AddLog(msg.c_str(), "info");
  _log_mutex.unlock();
}
void log_error(Window& win, const std::string& msg) {
  _log_mutex.lock();
  _log_widget.AddLog(msg.c_str(), "errn");
  _log_mutex.unlock();
}
void clear_log(Window& win) {
  _log_mutex.lock();
  _log_widget.Clear();
  _log_mutex.unlock();
}
void gui_log(Window& win) {
  _log_mutex.lock();
  _log_widget.Draw();
  _log_mutex.unlock();
}

}  // namespace opengl
