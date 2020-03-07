#version 330
out vec4 result;
in vec3 position;
in vec3 normal;

uniform mat4 frame;
uniform mat4 view;
uniform mat4 projection;

void main() {
    vec3 color = vec3(normal.y);
    color = color * 0.5 + vec3(0.5);
    result = vec4(color, 1);
}