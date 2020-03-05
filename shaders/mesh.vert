#version 330
layout(location = 0) in vec3 vposition;
layout(location = 1) in vec3 vnormal;
out vec3 position;
out vec3 normal;

uniform mat4 frame;
uniform mat4 view;
uniform mat4 projection;

void main() {
    vec4 p = projection * view * frame * vec4(vposition.xyz, 1);
    position = (frame * vec4(vposition, 1)).xyz;
    normal   = (frame * vec4(vnormal, 0)).xyz;
    gl_Position = projection * view * vec4(position, 1);
}