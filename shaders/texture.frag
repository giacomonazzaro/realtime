#version 330
in vec2 pos;
out vec4 result;

uniform sampler2D image;

void main() {
    vec2 uv = pos * 0.5 + 0.5;
    uv.y = 1 - uv.y;
    result = texture(image, uv);
    result.xyz = pow(result.xyz, vec3(1/2.2));
}
