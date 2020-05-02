#version 330
in vec2 pos;
out vec4 result;
uniform sampler2D color_tex;
uniform sampler2D normal_tex;

// vec4 blur9(sampler2D image, vec2 uv, vec2 resolution, vec2 direction) {
//   vec4 color = vec4(0.0);
//   vec2 off1 = vec2(1.3846153846) * direction;
//   vec2 off2 = vec2(3.2307692308) * direction;
//   color += texture(image, uv) * 0.2270270270;
//   color += texture(image, uv + (off1 / resolution)) * 0.3162162162;
//   color += texture(image, uv - (off1 / resolution)) * 0.3162162162;
//   color += texture(image, uv + (off2 / resolution)) * 0.0702702703;
//   color += texture(image, uv - (off2 / resolution)) * 0.0702702703;
//   return color;
// }

void main() {
    vec2 uv = (pos + 1) * 0.5;
    vec3 diffuse = texture(color_tex, uv).xyz;
    vec3 normal = vec3(0);
    vec2 eps = vec2(0.1, 0);
    float w = 0;
    normal = texture(normal_tex, uv).xyz;
    normal = normalize(2 * normal - 1);

    vec3 light = normalize(vec3(1,1,1));
    vec3 c = diffuse * max(dot(normal, light), 0);
    result = vec4(pow(c, vec3(1/2.2)), 1);  
}
