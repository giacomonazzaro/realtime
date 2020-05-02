#version 330
out vec4 result;
in vec3 position;
in vec3 normal;

uniform mat4 frame;
uniform mat4 view;
uniform mat4 projection;
uniform vec3 color;

void main() {
    vec3 diffuse = color;
    result = vec4(diffuse, 1);
}