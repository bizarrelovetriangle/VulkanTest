#version 450

layout(location = 0) in vec3 pos;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 texturePos;

layout(location = 0) out vec3 fragColor;

void main() {
    gl_Position = vec4(pos, 1.0);

    vec3 color = (gl_VertexIndex / 3) % 2 == 0 ? vec3(0.8, 0.8, 0.) : vec3(0., 0.8, 0.8);
    color = vec3(0.5);

    vec3 view = vec3(0., 0., 1.);
    fragColor = color * dot(view, normal);
}