#version 450

layout(location = 0) out vec3 outViewPosition;
layout(location = 1) out vec3 outViewNormal;
layout(location = 2) out vec3 outOrgPosition;

layout(binding = 0) uniform Uniform
{
	mat4x4 model;
	mat4x4 view;
	mat4x4 frustum;
} Transform;

vec3 normal = vec3(0., 1., 0.);
vec3 orgPositions[6] = vec3[](
	vec3(-1., 0.,  1.),
	vec3( 1., 0.,  1.),
	vec3( 1., 0., -1.),
		
	vec3( 1., 0., -1.),
	vec3(-1., 0., -1.),
	vec3(-1., 0.,  1.)
);

void main()
{
	vec3 orgPos = orgPositions[gl_VertexIndex] * 3;
	outOrgPosition = orgPos;

	mat4 viewSpace = Transform.view * Transform.model;
	vec3 viewPos = vec3(viewSpace * vec4(orgPos, 1.));
	outViewPosition = viewPos;

	mat3 normalMatrix = transpose(inverse(mat3(viewSpace)));
	outViewNormal = normalize(normalMatrix * normal);

	gl_Position = Transform.frustum * vec4(viewPos, 1.0);
}