#include "../source/ext/box2d-lite/include/box2d-lite/World.h"
#include "../source/yocto_opengl.h"
#include "../source/yocto_window.h"
using namespace yocto;
using namespace opengl;

enum struct atom_type { square = 0, triangle, pentagon };

struct Atom {
  vec2f     position = {0, 0};
  vec2f     rotation = {1, 0};
  vec2f     velocity = {0, 0};
  float     scale    = 1;
  atom_type type     = atom_type::square;
};

struct Game {
  vector<Atom> atoms;

  int selected = -1;

  vector<Shape> shapes;
  Shader        shader;
};

void move_atoms(Game& game, const Window& win) {
  float dt = win.input.time_delta;
  if (game.selected != -1) {
    auto& atom = game.atoms[game.selected];

    if (win.input.modifier_shift) {
      auto rot = rotation_mat(dt);
      if (is_key_pressed(win, Key::left)) atom.rotation = rot * atom.rotation;
      if (is_key_pressed(win, Key::right))
        atom.rotation = transpose(rot) * atom.rotation;
    } else {
      float s    = 0.3;
      vec2f step = {0, 0};
      if (is_key_pressed(win, Key::left)) step += {-1, 0};
      if (is_key_pressed(win, Key::right)) step += {1, 0};
      if (is_key_pressed(win, Key::up)) step += {0, 1};
      if (is_key_pressed(win, Key::down)) step += {0, -1};
      step          = normalize(step) * s;
      atom.velocity = step;
    }
  }

  for (int i = 0; i < game.atoms.size(); i++) {
    auto& atom = game.atoms[i];
    if (game.selected != i) atom.velocity *= yocto::exp(-dt);
    atom.position += atom.velocity * dt;
  }
}

float polygon_radius(int n) { return 2 * yocto::sin(pif / n); }

void draw(Window& win, Game& game) {
  clear_framebuffer({0.5, 0.2, 0.6, 1});

  move_atoms(game, win);

  // game.atoms[0].rotation = vec2f(
  //     cos(win.input.time_now), sin(win.input.time_now));
  // game.atoms[0].position = vec2f(cos(win.input.time_now), 0);

  for (int i = 0; i < game.atoms.size(); i++) {
    auto& atom = game.atoms[i];

    // clang-format off
    draw_shape(game.shapes[(int)atom.type], game.shader,
      Uniform("color", vec3f(1, 1, 1)),
      Uniform("position", atom.position),
      Uniform("rotation", atom.rotation),
      Uniform("scale", atom.scale),
      Uniform("id", i),
      Uniform("selected", game.selected),
      Uniform("viewport", win.framebuffer_size)
    );
    // clang-format on
  }
}

void init_game(Game& game) {
  game.shapes.resize(3);
  game.shapes[(int)atom_type::square]   = make_regular_polygon(4);
  game.shapes[(int)atom_type::triangle] = make_regular_polygon(3);
  game.shapes[(int)atom_type::pentagon] = make_regular_polygon(5);
  game.shader = create_shader("shaders/game.vert", "shaders/atom.frag", true);

  game.atoms.resize(2);
  game.atoms[0].position = vec2f(1, 0.5);
  game.atoms[0].scale    = 0.1;
  game.atoms[1].scale    = 0.1;

  // Physics
}

int main(int num_args, const char* args[]) {
  // Welcome
  auto game = Game{};
  auto win  = Window();

  win.callbacks.click = [&game](Window& win, bool left, bool pressed) {
    if (not pressed) return;
    int selected = -1;
    for (int i = 0; i < game.atoms.size(); i++) {
      auto& atom  = game.atoms[i];
      auto  mouse = get_mouse_pos_normalized(win, true);
      if (length(mouse - atom.position) < atom.scale) {
        selected = i;
      }
    }
    game.selected = selected;
  };

  win.callbacks.key = [&game](Window& win, Key key, bool pressed) {
    if (not pressed) return;
    if (game.selected != -1) {
      auto& atom = game.atoms[game.selected];
      if (key == Key('1')) atom.type = atom_type::square;
      if (key == Key('2')) atom.type = atom_type::triangle;
      if (key == Key('3')) atom.type = atom_type::pentagon;
    }

    if (key == Key(' ')) {
      auto atom     = Atom{};
      atom.scale    = 0.1;
      game.selected = game.atoms.size();
      game.atoms.push_back(atom);
    }
  };

  init_window(win, {800, 600}, "Symchem");
  set_blending(true);
  init_game(game);
  run_draw_loop(win, [&game](Window& win) { draw(win, game); });
}
