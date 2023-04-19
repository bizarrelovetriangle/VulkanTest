#version 450

layout(location = 0) in vec3 pos;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 texturePos;
layout(location = 3) in vec4 color;

layout(location = 0) out float fragFactor;
layout(location = 1) out vec4 fragColor;
layout(location = 2) out vec2 fragTexturePos;

layout (push_constant) uniform constans
{
	mat4x4 model;
	mat4x4 world;
} Matrixes;

void main()
{
	mat4 matrix = Matrixes.world * Matrixes.model;
	gl_Position = matrix * vec4(pos, 1.0);
	gl_Position.z /= 10;

	vec3 rotatedNormal = vec3(matrix * vec4(normal, 0.));

	vec3 view = vec3(0., 0., -1.);
	fragFactor = dot(view, rotatedNormal);
	fragColor = color;
	fragTexturePos = texturePos;
}