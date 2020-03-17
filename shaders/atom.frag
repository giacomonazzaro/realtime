#version 330
out vec4 result;
uniform vec3 color;
in float value;
uniform float scale;
uniform int selected;
uniform int id;

void main() {
    float border = smoothstep(0.0, 0.02 / scale, 1 - value);
    vec3 c = mix(vec3(0), color, border);
    result = vec4(c, 1);
    if(id == selected) result = vec4(1,0,0,1);
}
