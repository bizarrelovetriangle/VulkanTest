#version 450

layout(location = 0) in vec3 pos;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 texturePos;

layout(location = 0) out float fragFactor;
layout(location = 1) out vec2 fragTexturePos;

layout(binding = 0) uniform Uniform
{
	mat4x4 model;
	mat4x4 view;
	mat4x4 frustum;
} Transform;

void main()
{
	mat4 viewSpace = Transform.view * Transform.model;

	mat3 normalMatrix = transpose(inverse(mat3(viewSpace)));
	vec3 rotatedNormal = normalMatrix * normal;
	rotatedNormal = normalize(rotatedNormal);

	vec3 viewPos = vec3(viewSpace * vec4(pos, 1.));
	vec3 lightPos = vec3(0, 0, 0);
	vec3 negLightDir = normalize(lightPos - viewPos);
	fragFactor = dot(negLightDir, rotatedNormal);

	fragTexturePos = texturePos;

	gl_Position = Transform.frustum * viewSpace * vec4(pos, 1.0);
}