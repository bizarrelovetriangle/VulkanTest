#version 450

layout(location = 0) out vec3 outPosition;
layout(location = 1) out float outFactor;

layout (push_constant) uniform constans
{
	mat4x4 model;
	mat4x4 world;
} Matrixes;

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

	vec3 pos = positions[gl_VertexIndex];
	mat4 matrix = Matrixes.world * Matrixes.model;

	vec3 rotatedNormal = vec3(matrix * vec4(normal, 0.));
	vec3 view = vec3(0., 0., -1.);
	outFactor = dot(view, rotatedNormal);

	outPosition = pos;
	gl_Position = matrix * vec4(pos, 1.0);
	
	gl_Position.x /= gl_Position.z;
	gl_Position.y /= gl_Position.z;

	gl_Position.z /= 10;
	//gl_Position /= gl_Position.z;
}