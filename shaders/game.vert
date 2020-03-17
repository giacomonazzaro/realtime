#version 330
layout(location = 0) in vec2 vpos;
layout(location = 1) in float vval;

uniform ivec2 viewport;
uniform vec2 position;
uniform vec2 rotation;
uniform float scale;

out vec2 pos;
out float value;

void main() {
    pos = vpos;
    mat2 rot = mat2(rotation.x, rotation.y, -rotation.y, rotation.x);
    pos = rot * pos;
    pos *= scale;
    pos += position;
    pos.x *= float(viewport.y) / viewport.x;
    value = vval;
    gl_Position = vec4(pos, 0, 1);
}