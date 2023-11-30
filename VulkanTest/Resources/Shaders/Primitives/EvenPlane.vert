#version 450

layout(location = 0) out vec3 outPosition;
layout(location = 1) out float outFactor;

layout(binding = 0) uniform Uniform
{
	mat4x4 model;
	mat4x4 world;
	mat4x4 view;
} Transform;

void main()
{
	vec3 normal = vec3(0., 1., 0.);
	vec3 positions[6] = vec3[](
		vec3(-1., 0.,  1.),
		vec3( 1., 0.,  1.),
		vec3( 1., 0., -1.),
		
		vec3( 1., 0., -1.),
		vec3(-1., 0., -1.),
		vec3(-1., 0.,  1.)
	);

	vec3 pos = positions[gl_VertexIndex] * 3;
	mat4 matrix = Transform.world * Transform.model;

	vec3 rotatedNormal = mat3x3(matrix) * normal;
	//vec3 view = vec3(0., 0., 1.);
	//outFactor = dot(view, rotatedNormal);
	outFactor = 0;

	outPosition = pos;
	gl_Position = matrix * vec4(pos, 1.0);
	
	gl_Position.x /= gl_Position.z;
	gl_Position.y /= gl_Position.z;

	gl_Position.z /= 10;
}