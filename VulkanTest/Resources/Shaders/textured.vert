#version 450

layout(location = 0) in vec3 pos;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 texturePos;

layout(location = 0) out vec3 outViewPosition;
layout(location = 1) out vec3 outViewNormal;
layout(location = 2) out vec2 outTexturePos;

layout(binding = 0) uniform Uniform
{
	mat4x4 modelToWorld;
	mat4x4 worldToView;
	mat4x4 viewToProj;
} Transform;

void main()
{
	mat4 modelToView = Transform.worldToView * Transform.modelToWorld;
	vec3 viewPos = vec3(modelToView * vec4(pos, 1.));
	outViewPosition = viewPos;

	mat3 normalMatrix = transpose(inverse(mat3(modelToView)));
	outViewNormal = normalize(normalMatrix * normal);

	outTexturePos = texturePos;

	gl_Position = Transform.viewToProj * vec4(viewPos, 1.0);
}