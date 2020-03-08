#version 330
layout(location = 0) in vec2 vpos;
out vec2 pos;
void main() {
    pos = vpos;
    gl_Position = vec4(vpos.xy, 0, 1);
}