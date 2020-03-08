#version 330
out vec4 result;
uniform vec3 color;

void main() {
    result = vec4(color, 1);
}
