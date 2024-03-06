#version 450

layout(location = 0) in vec3 pos;
layout(location = 1) in vec4 color;

layout(location = 0) out vec4 outColor;

layout(binding = 0) uniform Uniform
{
	mat4x4 model;
	mat4x4 view;
	mat4x4 frustum;
} Transform;

void main()
{
	mat4 viewSpace = Transform.view * Transform.model;
	vec3 viewPos = vec3(viewSpace * vec4(pos, 1.));

	outColor = color;

	gl_Position = Transform.frustum * vec4(viewPos, 1.0);
}