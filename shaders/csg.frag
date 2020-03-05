#version 330
out vec4 color;
in vec3  position;

uniform mat4 frame;
uniform mat4 view;
uniform mat4 projection;
uniform vec3 eye;

struct CsgNode {
  ivec2 children;
  float params[6];
  int   type;
};

uniform int num_nodes;
uniform CsgNode nodes[64];

float smin(float a, float b, float k) {
  if (k == 0) return min(a, b);
  float h = max(k - abs(a - b), 0.0) / k;
  return min(a, b) - h * h * k * (1.0 / 4.0);
}

float smax(float a, float b, float k) {
  if (k == 0) return max(a, b);
  float h = max(k - abs(a - b), 0.0) / k;
  return max(a, b) + h * h * k * (1.0 / 4.0);
}

float eval_primitive(vec3 position, in CsgNode node) {
  // Sphere
  // if (type == primitive_type::sphere) {
    vec3 center = vec3(node.params[0], node.params[1], node.params[2]);
    float radius = node.params[3];
    return length(position - center) - radius;
  // }
}

float eval_operation(float f, float g, in CsgNode node) {
  if (node.params[0] >= 0) {
    // Union
    return mix(f, smin(f, g, node.params[1]), node.params[0]);
  } else {
    // Subtracion
    return mix(f, smax(f, -g, node.params[1]), -node.params[0]);
  }
}

float values[128];
float sdf(vec3 position) {
  for (int i = 0; i < num_nodes; i++) {
    if (nodes[i].children == ivec2(-1, -1)) {
      values[i] = eval_primitive(position, nodes[i]);
    } else {
      float f    = values[nodes[i].children.x];
      float g    = values[nodes[i].children.y];
      values[i] = eval_operation(f, g, nodes[i]);
    }
  }
  return values[num_nodes - 1];
}

vec3 compute_normal(vec3 p) {
    float eps = 0.001;
    float o   = sdf(p);
    float x   = sdf(p + vec3(eps, 0, 0));
    float y   = sdf(p + vec3(0, eps, 0));
    float z   = sdf(p + vec3(0, 0, eps));
    return normalize(vec3(x, y, z) - vec3(o));
}

void main() {
  vec3 p  = position - vec3(0.5);
  // color = vec4(p, 1);
  // return;
  vec3 rd = normalize(position - eye);
  for (int i = 0; i < 10; i++) {
    float f = sdf(p);
    if (abs(f) < 0.01) {
      vec3 normal = compute_normal(p);
      color = vec4(normal, 1);
      return;
    }
    p += f * rd;
  }
  color = vec4(0);
}