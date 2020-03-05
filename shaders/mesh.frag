#version 330
out vec4 color;
in vec3 position;
in vec3 normal;

uniform mat4 frame;
uniform mat4 view;
uniform mat4 projection;

void main() {
    color = vec4(vec3(normal.y) + 0.5, 1);
}