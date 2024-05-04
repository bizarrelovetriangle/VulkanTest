#version 450

layout(location = 0) in vec3 pos;
layout(location = 1) in vec4 color;

layout(location = 0) out vec4 outColor;

layout(binding = 0) uniform CommonUniform
{
	mat4x4 worldToView;
	mat4x4 viewToProj;
} Common;

layout(binding = 1) uniform TransformUniform
{
	mat4x4 modelToWorld;
} Transform;

void main()
{
	mat4 viewSpace = Common.worldToView * Transform.modelToWorld;
	vec3 viewPos = vec3(viewSpace * vec4(pos, 1.));

	outColor = color;

	gl_Position = Common.viewToProj * vec4(viewPos, 1.0);
}