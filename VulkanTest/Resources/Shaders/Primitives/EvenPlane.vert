#version 450
layout(location = 0) in vec3 pos;
layout(location = 1) in vec3 normal;

layout(location = 0) out vec3 outViewPosition;
layout(location = 1) out vec3 outViewNormal;
layout(location = 2) out vec3 outOrgPosition;

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
	outOrgPosition = pos;

	mat4 modelToView = Common.worldToView * Transform.modelToWorld;
	vec3 viewPos = vec3(modelToView * vec4(pos, 1.));
	outViewPosition = viewPos;

	mat3 normalMatrix = transpose(inverse(mat3(modelToView)));
	outViewNormal = normalize(normalMatrix * normal);

	gl_Position = Common.viewToProj * vec4(viewPos, 1.0);
}