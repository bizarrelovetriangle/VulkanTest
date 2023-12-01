#version 450

layout(location = 0) out vec3 outPosition;
layout(location = 1) out float outFactor;

layout(binding = 0) uniform Uniform
{
	mat4x4 model;
	mat4x4 view;
	mat4x4 frustum;
} Transform;

vec3 normal = vec3(0., 1., 0.);
vec3 positions[6] = vec3[](
	vec3(-1., 0.,  1.),
	vec3( 1., 0.,  1.),
	vec3( 1., 0., -1.),
		
	vec3( 1., 0., -1.),
	vec3(-1., 0., -1.),
	vec3(-1., 0.,  1.)
);

void main()
{
	vec3 pos = positions[gl_VertexIndex] * 3;
	mat4 viewSpace = Transform.view * Transform.model;

	mat3 normalMatrix = transpose(inverse(mat3(viewSpace)));
	vec3 rotatedNormal = normalMatrix * normal;
	rotatedNormal = normalize(rotatedNormal);

	vec3 viewPos = vec3(viewSpace * vec4(pos, 1.));
	vec3 lightPos = vec3(0, 0, 0);
	vec3 negLightDir = normalize(lightPos - viewPos);
	outFactor = dot(negLightDir, rotatedNormal);

	if (gl_VertexIndex < 3){
		//outFactor = 1;
	}

	outPosition = pos;
	gl_Position = Transform.frustum * viewSpace * vec4(pos, 1.0);
}