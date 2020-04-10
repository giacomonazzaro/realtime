#include <GLFW/glfw3.h>

#include <iostream>

#include "../source/ext/box2d/include/box2d/box2d.h"
#include "../source/yocto_opengl.h"
#include "../source/yocto_window.h"

using namespace yocto;
using namespace opengl;

enum struct atom_type { square = 0, triangle, pentagon, circle };

struct Atom {
  float     scale = 1;
  atom_type type  = atom_type::square;
  b2Body*   body;

  vec2f position() const { return vec2f(&body->GetPosition()); }
  vec2f rotation() const { return vec2f(&body->GetTransform().q); }
};

struct Molecule {
  vector<Atom>          atoms;
  vector<array<int, 5>> graph;
  b2Body*               body;
};

void bond(Molecule& molecule, const Atom& atom, int node) {}

struct Game {
  vector<Atom> atoms;

  int selected = -1;

  vector<Shape> shapes;
  Shader        shader;

  b2World world = b2World({0, 0});
};

vector<vec2f> make_regular_polygon_(int N) {
  // if (N == 4) return {{0.5, 0.5}, {-0.5, 0.5}, {-0.5, -0.5}, {0.5, -0.5}};
  auto positions = vector<vec2f>(N);
  for (int i = 0; i < N; ++i) {
    float angle  = (2 * pif * i) / N;
    positions[i] = {yocto::cos(angle), yocto::sin(angle)};
    positions[i] /= 2 * yocto::sin(pif / N);
  }
  return positions;
}

Atom& add_atom(Game& game, atom_type type = atom_type::square,
    const vec2f& position = {0, 0}, float scale = 0.1) {
  game.atoms.push_back({});
  auto& atom = game.atoms.back();
  atom.type  = type;
  atom.scale = scale;
  b2BodyDef bodyDef;
  bodyDef.type = b2_dynamicBody;
  bodyDef.position.Set(position.x, position.y);

  if (atom.type == atom_type::circle) {
    atom.body = game.world.CreateBody(&bodyDef);

    b2CircleShape circle;
    circle.m_radius = atom.scale;

    b2FixtureDef fixtureDef;
    fixtureDef.shape       = &circle;
    fixtureDef.density     = 1.0f;
    fixtureDef.friction    = 0.3f;
    fixtureDef.restitution = 0.5f;

    atom.body->CreateFixture(&fixtureDef);
    atom.body->SetAngularDamping(10.5);

    return atom;
  }

  int num_sides;
  if (atom.type == atom_type::square) num_sides = 4;
  if (atom.type == atom_type::triangle) num_sides = 3;
  if (atom.type == atom_type::pentagon) num_sides = 5;
  auto positions = make_regular_polygon_(num_sides);

  atom.body = game.world.CreateBody(&bodyDef);
  b2PolygonShape polygon;
  b2Vec2         vertices[12];
  for (int i = 0; i < num_sides; i++) {
    vertices[i].Set(positions[i].x * atom.scale, positions[i].y * atom.scale);
  }
  polygon.Set(vertices, num_sides);

  b2FixtureDef fixtureDef;
  fixtureDef.shape       = &polygon;
  fixtureDef.density     = 1.0f;
  fixtureDef.friction    = 0.3f;
  fixtureDef.restitution = 0.5f;

  atom.body->CreateFixture(&fixtureDef);
  atom.body->SetAngularDamping(0.5);

  return atom;
}

void move_atoms(Game& game, const Window& win) {
  float dt = win.input.time_delta;
  if (game.selected != -1) {
    auto& atom = game.atoms[game.selected];

    {
      auto& joystick = win.joysticks[0];

      int          axesCount;
      const float* axes = glfwGetJoystickAxes(GLFW_JOYSTICK_1, &axesCount);

      auto lstick = joystick.left_stick;
      auto rstick = (joystick.right_stick);

      if (length(lstick) < 0.05) lstick = {0, 0};
      if (length(rstick) < 0.05) rstick = {0, 0};
      atom.body->SetLinearVelocity({lstick.x, lstick.y});

      auto rot = atom.rotation();
      auto a   = cross(rot, rstick);
      // printf("%f %f\n", rstick.x, rstick.y);
      atom.body->SetAngularVelocity(a);

      // if (joystick.cross()) printf("cross\n");
      // if (joystick.square()) printf("square\n");
      // if (joystick.triangle()) printf("triangle\n");
      // if (joystick.circle()) printf("circle\n");
      // if (joystick.left_bumper()) printf("left_bumper\n");
      // if (joystick.right_bumper()) printf("right_bumper\n");

      // atom.body->m_xf.q = b2Rot(rstick.x, rstick.y);

      // atom.body->ApplyForceToCenter({force.x, force.y}, true);

      // std::cout << "Trigs: " << axes[3] << " " << axes[4] << std::endl
      //           << std::endl;
    }

    // if (win.input.modifier_shift) {
    //   // auto rot = rotation_mat(dt * 2);
    //   // if (is_key_pressed(win, Key::left))
    //   //   atom.rotation() = rot * atom.rotation();
    //   // if (is_key_pressed(win, Key::right))
    //   //   atom.rotation() = transpose(rot) * atom.rotation();
    // } else {
    //   float s    = 0.3;
    //   vec2f step = {0, 0};
    //   if (is_key_pressed(win, Key::left)) step += {-1, 0};
    //   if (is_key_pressed(win, Key::right)) step += {1, 0};
    //   if (is_key_pressed(win, Key::up)) step += {0, 1};
    //   if (is_key_pressed(win, Key::down)) step += {0, -1};
    //   step = normalize(step) * s;
    //   atom.body->SetLinearVelocity({step.x, step.y});
    // }

    // if (win.input.mouse_left) {
    //   auto delta = win.input.mouse_pos - win.input.mouse_last;
    //   delta.y *= -1;
    //   atom.body->SetLinearVelocity({delta.x, delta.y});
    // }

    if (win.input.mouse_right) {
      auto  mouse = get_mouse_pos_normalized(win, true);
      auto& atom  = game.atoms[game.selected];
      auto  force = normalize(mouse - atom.position());
      force *= 0.02;
      atom.body->ApplyForceToCenter({force.x, force.y}, true);
    }
  }

  float timeStep           = 1.0f / 120.0f;
  int   velocityIterations = 8;
  int   positionIterations = 8;
  game.world.Step(timeStep, velocityIterations, positionIterations);
}

float polygon_radius(int n) { return 2 * yocto::sin(pif / n); }

void draw(Window& win, Game& game) {
  if (not win.input.is_window_focused) return;

  clear_framebuffer({0.5, 0.2, 0.6, 1});

  move_atoms(game, win);

  for (int i = 0; i < game.atoms.size(); i++) {
    auto& atom  = game.atoms[i];
    auto  frame = atom.body->GetTransform();

    // clang-format off
    draw_shape(game.shapes[(int)atom.type], game.shader,
      Uniform("color", vec3f(1, 1, 1)),
      Uniform("position", vec2f(&frame.p)),
      Uniform("rotation", vec2f(&frame.q)),
      Uniform("scale", atom.scale * 1.05f),
      Uniform("id", i),
      Uniform("type", (int)atom.type),
      Uniform("selected", game.selected),
      Uniform("viewport", win.framebuffer_size)
    );
    // clang-format on
  }
}

void init_game(Game& game) {
  game.shapes.resize(4);
  game.shapes[(int)atom_type::square]   = make_regular_polygon(4);
  game.shapes[(int)atom_type::triangle] = make_regular_polygon(3);
  game.shapes[(int)atom_type::pentagon] = make_regular_polygon(5);
  game.shapes[(int)atom_type::circle]   = make_quad();
  game.shader = create_shader("shaders/game.vert", "shaders/atom.frag", true);

  add_atom(game, atom_type::pentagon, {0, 0}, 0.3);
  add_atom(game, atom_type::triangle, {0.1, 0.5}, 0.3);
  add_atom(game, atom_type::circle, {-0.3, -0.3}, 0.2);

  vector<b2Vec2> vertices = {{1, 1}, {-1, 1}, {-1, -1}, {1, -1}};
  for (int i = 0; i < 4; i++) {
    b2BodyDef bodyDef;
    auto      body = game.world.CreateBody(&bodyDef);

    b2EdgeShape edge;
    edge.Set(vertices[i], vertices[(i + 1) % 4]);
    b2FixtureDef fixtureDef;
    fixtureDef.shape       = &edge;
    fixtureDef.density     = 1.0f;
    fixtureDef.friction    = 1.0f;
    fixtureDef.restitution = 0.5f;
    body->CreateFixture(&fixtureDef);
  }
}

int main(int num_args, const char* args[]) {
  // Welcome
  auto game = Game{};
  auto win  = Window();

  win.callbacks.click = [&game](Window& win, bool left, bool pressed) {
    if (not pressed) return;
    auto mouse = get_mouse_pos_normalized(win, true);

    if (left) {
      int selected = -1;
      for (int i = 0; i < game.atoms.size(); i++) {
        auto& atom = game.atoms[i];
        for (auto fixture = atom.body->GetFixtureList(); fixture;
             fixture      = fixture->GetNext()) {
          if (fixture->TestPoint({mouse.x, mouse.y})) {
            selected = i;
          }
        }
        // if (atom->body)
        // if (length(mouse - atom.position()) < atom.scale) {
        // selected = i;
        // }
      }
      game.selected = selected;
    }
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
      atom.scale    = 0.3;
      game.selected = game.atoms.size();
      game.atoms.push_back(atom);
    }
  };

  init_window(win, {800, 800}, "Symchem");
  set_blending(true);
  init_game(game);
  run_draw_loop(win, [&game](Window& win) { draw(win, game); });
}
