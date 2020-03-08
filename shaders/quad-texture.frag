#version 330
in vec2 pos;
out vec4 result;

uniform sampler2D color_tex;
uniform vec3 color;

void main() {
    vec2 uv = (pos + 1) * 0.5;
    // vec3 c = vec3(uv, 0);
    vec3 c = vec3(0);
    vec2 eps = vec2(0.01, 0);

    c += pow(texture(color_tex, uv+eps.xy).xyz, vec3(2.2));
    c += pow(texture(color_tex, uv+eps.yx).xyz, vec3(2.2));
    c += pow(texture(color_tex, uv+eps.xx).xyz, vec3(2.2));
    c += pow(texture(color_tex, uv+eps.yy).xyz, vec3(2.2));

    c /= 4;

    result = vec4(color * c, 1);
}
