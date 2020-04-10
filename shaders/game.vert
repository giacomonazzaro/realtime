#version 330
layout(location = 0) in vec2 vpos;
// layout(location = 1) in float vval;

uniform ivec2 viewport;
uniform vec2 position;
uniform vec2 rotation;
uniform float scale;
uniform int type;

out vec2 pos;
out float value;

// square = 0, triangle, pentagon, circle };

void main() {
    pos = vpos;
    mat2 rot = mat2(rotation.x, rotation.y, -rotation.y, rotation.x);
    vec2 pos = rot * vpos;
    pos *= scale;
    pos += position;
    pos.x *= float(viewport.y) / viewport.x;
    value = length(vpos);
    if(type == 0) value *= 2 * sin(3.1416 / 4);
    if(type == 1) value *= 2 * sin(3.1416 / 3);
    if(type == 2) value *= 2 * sin(3.1416 / 5);
    gl_Position = vec4(pos, 0, 1);
}