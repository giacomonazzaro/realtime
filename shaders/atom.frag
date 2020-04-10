#version 330
in vec2 pos;
in float value;
out vec4 result;

uniform vec3 color;
uniform float scale;
uniform int selected;
uniform int id;
uniform int type;

void main() {
    float v = value;
    if(type == 3) {
        v = length(pos);
    }
    float alpha = 1 - smoothstep(0.99, 1.0, v);
    float border = 1 - smoothstep(0.9, 1.0, v);
    vec3 fill = color;
    if(id == selected) fill *= vec3(1, 0.5, 0.5);
    result = vec4(fill * border, alpha);
}
